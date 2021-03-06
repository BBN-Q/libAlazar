/*
Original author: Rob McGurrin

Copyright 2016-2017 Raytheon BBN Technologies
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <thread>
#include <time.h>
#include <cstring>
#include <cmath>
#include <mutex>

std::mutex mu;

#ifndef _WIN32
  #include <sys/socket.h>
#else
  #include <winsock2.h>
  #include <basetsd.h>
  typedef SSIZE_T ssize_t;
#endif

#include "libAlazar.h"
#include "libAlazarAPI.h"
#include <plog/Log.h>

using namespace std;

uint32_t systemCount() {
  return AlazarNumOfSystems();
}

uint32_t boardCount(uint32_t systemId) {
  return AlazarBoardsInSystemBySystemID(systemId);
}

std::string boardInfo(uint32_t systemId, uint32_t boardId) {
  std::ostringstream ss;
  uint32_t serialNumber;
  HANDLE handle = AlazarGetBoardBySystemID(systemId, boardId);
  RETURN_CODE retCode = AlazarQueryCapability(handle, GET_SERIAL_NUMBER, 0, &serialNumber);

  if (retCode != ApiSuccess) {
    LOG(plog::error) << "AlazarQueryCapability failed -- "
                       << AlazarErrorToText(retCode);
    ss << "Alazar ATS9870 UnknownSerial";
  } else {
    ss << "Alazar ATS9870 " << to_string(serialNumber);
  }
  return ss.str();
}


AlazarATS9870::AlazarATS9870() : threadStop(false), threadRunning(false) {
  LOG(plog::verbose) << "Constructing ... ";
}

AlazarATS9870::~AlazarATS9870() {
  LOG(plog::verbose) << "Destructing ...";

  RETURN_CODE retCode = AlazarCloseAUTODma(boardHandle);
  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
  }

  AlazarClose(boardHandle);
}

int32_t AlazarATS9870::ConfigureBoard(uint32_t systemId, uint32_t boardId,
                                      const ConfigData_t &config,
                                      AcquisitionParams_t &acqParams) {

  boardHandle = AlazarGetBoardBySystemID(systemId, boardId);
  if (boardHandle == NULL) {
    LOG(plog::error) << "Open systemId " << systemId << " boardId " << boardId
                       << " failed";
    return -1;
  }

  // set averager mode or digitizer mode
  const char *acquireModeKey = config.acquireMode;
  if (modeMap.find(acquireModeKey) == modeMap.end()) {
    LOG(plog::error) << "Invalid Mode: " << acquireModeKey;
    return (-1);
  }
  averager = modeMap[config.acquireMode];

  // set the sample rate parameters:
  // SampleRateId is set to 1e9 and there is an external ref clock configured
  // so the sample rate is 1e9/decimation; decimation factor has to be 1,2,4
  // or any multiple of 10
  uint32_t decimation = 1000000000 / static_cast<uint32_t>(config.samplingRate);
  LOG(plog::info) << "Decimation " << decimation;
  if (decimation != 1 && decimation != 2 && decimation != 4) {
    if (decimation % 10 != 0) {
      LOG(plog::error) << "Decimation is not a Mulitple of 2,4,or 10 ";
      return (-1);
    }
  }

  RETURN_CODE retCode =
      AlazarSetCaptureClock(boardHandle,              // HANDLE -- board handle
                            EXTERNAL_CLOCK_10MHz_REF, // U32 -- clock source id
                            1000000000,        // U32 -- sample rate id - 1e9
                            CLOCK_EDGE_RISING, // U32 -- clock edge id
                            decimation         // U32 -- clock decimation
                            );
  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
    return -1;
  }

  // set up the channel parameters for channel A
  channelScale = config.verticalScale;
  channelOffset = config.verticalOffset;
  counts2Volts = 2 * channelScale / 256.0;

  uint32_t rangeIDKey = static_cast<uint32_t>(std::lround(config.verticalScale * 1000));
  if (rangeIdMap.find(rangeIDKey) == rangeIdMap.end()) {
    LOG(plog::error) << "Invalid Channel Scale: " << rangeIDKey;
    return (-1);
  }

  const char *couplingKey = config.verticalCoupling;
  if (couplingMap.find(couplingKey) == couplingMap.end()) {
    LOG(plog::error) << "Invalid Channel Coupling: " << couplingKey;
    return (-1);
  }

  LOG(plog::info) << "Input Range: " << channelScale
                    << " ID: " << rangeIdMap[rangeIDKey];
  LOG(plog::info) << "Counts2Volts: " << counts2Volts;

  retCode =
      AlazarInputControl(boardHandle,              // HANDLE -- board handle
                         CHANNEL_A,                // U8 -- input channel
                         couplingMap[couplingKey], // U32 -- input coupling id
                         // TODO verify values for vertical scale
                         rangeIdMap[rangeIDKey], // U32 -- input range id
                         IMPEDANCE_50_OHM        // U32 -- input impedance id
                         );
  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
    return -1;
  }

  const char *bandwidthKey = config.bandwidth;
  if (bandwidthMap.find(bandwidthKey) == bandwidthMap.end()) {
    LOG(plog::error) << "Invalid Mode: " << bandwidthKey;
    return (-1);
  }
  retCode = AlazarSetBWLimit(
      boardHandle,                  // HANDLE -- board handle
      CHANNEL_A,                    // U8 -- channel identifier
      bandwidthMap[config.bandwidth] // U32 -- 0 = disable, 1 = enable
      );
  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
    return -1;
  }

  // set up the channel parameters for channel B
  retCode = AlazarInputControl(
      boardHandle,                          // HANDLE -- board handle
      CHANNEL_B,                            // U8 -- input channel
      couplingMap[config.verticalCoupling], // U32 -- input coupling id
      rangeIdMap[rangeIDKey],               // U32 -- input range id
      IMPEDANCE_50_OHM                      // U32 -- input impedance id
      );
  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
    return -1;
  }

  retCode = AlazarSetBWLimit(
      boardHandle,                  // HANDLE -- board handle
      CHANNEL_B,                    // U8 -- channel identifier
      bandwidthMap[config.bandwidth] // U32 -- 0 = disable, 1 = enable
      );
  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
    return -1;
  }

  // Select trigger inputs and levels as required
  // trigLevelCode = uint8(128 +
  // 127*(trigSettings.triggerLevel/1000/trigChannelRange));
  // uint32_t = conf->triggerLevel
  uint32_t trigChannelRange = 5;
  uint32_t trigLevelCode = static_cast<uint32_t>(
      128 + 127 * (config.triggerLevel / 1000 / trigChannelRange));
  LOG(plog::info) << "Trigger Level Code " << trigLevelCode;

  const char *triggerSourceKey = config.triggerSource;
  if (triggerSourceMap.find(triggerSourceKey) == triggerSourceMap.end()) {
    LOG(plog::error) << "Invalid Trigger Source ID: " << triggerSourceKey;
    return (-1);
  }

  const char *triggerSlopeMapKey = config.triggerSlope;
  if (triggerSlopeMap.find(triggerSlopeMapKey) == triggerSlopeMap.end()) {
    LOG(plog::error) << "Invalid Trigger Coupling: " << triggerSlopeMapKey;
    return (-1);
  }

  retCode = AlazarSetTriggerOperation(
      boardHandle,                            // HANDLE -- board handle
      TRIG_ENGINE_OP_J,                       // U32 -- trigger operation
      TRIG_ENGINE_J,                          // U32 -- trigger engine id
      triggerSourceMap[config.triggerSource], // U32 -- trigger source id
      triggerSlopeMap[config.triggerSlope],   // U32 -- trigger slope id
      trigLevelCode, // U32 -- trigger level from 0 (-range) to 255 (+range)
      TRIG_ENGINE_K, // U32 -- trigger engine id
      TRIG_DISABLE,  // U32 -- trigger source id for engine K
      TRIGGER_SLOPE_POSITIVE, // U32 -- trigger slope id
      128 // U32 -- trigger level from 0 (-range) to 255 (+range)
      );
  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
    return -1;
  }

  // set up external triggerCoupling
  // TODO - add channel based triggering
  couplingKey = config.triggerCoupling;
  if (couplingMap.find(couplingKey) == couplingMap.end()) {
    LOG(plog::error) << "Invalid Trigger Coupling: " << couplingKey;
    return (-1);
  }

  retCode = AlazarSetExternalTrigger(
      boardHandle,                         // HANDLE -- board handle
      couplingMap[config.triggerCoupling], // U32 -- external trigger coupling
                                           // id
      ETR_5V                               // U32 -- external trigger range id
      );
  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
    return -1;
  }

  // set the trigger delay in samples
  uint32_t trigDelayPts = config.samplingRate * config.delay;
  LOG(plog::info) << "Trigger Delay " << trigDelayPts;
  retCode = AlazarSetTriggerDelay(boardHandle, trigDelayPts);
  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
    return -1;
  }

  // set timeout to 0 - don't time out waiting for a trigger
  retCode = AlazarSetTriggerTimeOut(
      boardHandle, // HANDLE -- board handle
      0            // U32 -- timeout_sec / 10.e-6 (0 means wait forever)
      );
  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
    return -1;
  }

  recordLength = config.recordLength;

  if (recordLength < 256) {
    LOG(plog::error) << "recordLength less than 256";
    return -1;
  }

  if (recordLength % 64 != 0) {
    LOG(plog::error) << "recordLength is not aligned on a 64 sample boundary";
    return -1;
  }

  LOG(plog::info) << "recordLength: " << recordLength;
  retCode = AlazarSetRecordSize(boardHandle, 0, recordLength);
  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
  }

  nbrSegments = config.nbrSegments;
  nbrWaveforms = config.nbrWaveforms;
  nbrRoundRobins = config.nbrRoundRobins;

  // compute records per buffer and records per acquisition
  if (getBufferSize() < 0) {
    return (-1);
  }

  // The application uses this info allocate its channel data buffers
  if (!partialBuffer) {
    if (averager) {
      acqParams.samplesPerAcquisition = recordLength * nbrSegments;
    } else {
      acqParams.samplesPerAcquisition =
          recordLength * nbrSegments * nbrWaveforms * roundRobinsPerBuffer;
    }
    acqParams.numberAcquisitions = nbrBuffers;
  } else {
    if (averager) {
      acqParams.samplesPerAcquisition = recordLength * nbrSegments;
    } else {
      acqParams.samplesPerAcquisition =
          recordLength * nbrSegments * nbrWaveforms;
    }
    acqParams.numberAcquisitions = nbrBuffers / buffersPerRoundRobin;
  }

  samplesPerAcquisition = acqParams.samplesPerAcquisition;
  LOG(plog::info) << "samplesPerAcquisition: " << samplesPerAcquisition;
  LOG(plog::info) << "numberAcquisitions: " << acqParams.numberAcquisitions;

  ch1WorkBuff.resize(samplesPerAcquisition);
  ch2WorkBuff.resize(samplesPerAcquisition);

  uint32_t m = sizeof(socketbuffsize);
  #ifdef _WIN32
    socketbuffsize = 65536; // placeholder for now
    size_t num_floats = socketbuffsize/sizeof(float);
  #else
    int32_t fdsocket = socket(AF_UNIX,SOCK_STREAM,0);
    getsockopt(fdsocket,SOL_SOCKET,SO_RCVBUF,(void *)&socketbuffsize, &m);
    size_t num_floats = socketbuffsize/sizeof(float);
    socketbuffsize = (num_floats-1)*sizeof(float);
    close(fdsocket);
  #endif
    LOG(plog::info) << "num floats per send: " << num_floats;
    LOG(plog::info) << "socket buffer size: " << socketbuffsize;
  return 0;
}

int32_t AlazarATS9870::rx(int32_t* ready) {
  RETURN_CODE retCode;
  uint32_t count = 0;
  LOG(plog::verbose) << "STARTING RX THREAD";

  mu.lock();
  *ready = 1;
  mu.unlock();

  while (bufferCounter < static_cast<int32_t>(nbrBuffers)) {
    std::shared_ptr<std::vector<uint8_t>> buff;
    while (!bufferQ.pop(buff)) {
      if (threadStop) {
        return 0;
      }
    }

    while (1) {

      if (threadStop) {
        return 0;
      }
      retCode = AlazarWaitAsyncBufferComplete(boardHandle,
                                              buff.get()->data(),
                                              1000); // 1 sec timeout
      if (retCode == ApiWaitTimeout) {
        continue;
      } else if (retCode == ApiSuccess) {
        LOG(plog::verbose) << "GOT BUFFER " << count++;
        break;
      } else {
        printError(retCode, __FILE__, __LINE__);
        return -1;
      }
    }

    // if we have a socket, process the data and send it when we have a full
    // buffer
    if (sockets[0] != -1 || sockets[1] != -1) {
      int32_t full = processBuffer(
                       buff,
                       ch1WorkBuff.data(),
                       ch2WorkBuff.data()
                     );
      LOG(plog::verbose) << "Full Attempting to write to socket, full: " << full;
      if (full) {
        ssize_t status;
        LOG(plog::verbose) << "Work buff size: " << ch1WorkBuff.size();
        size_t buf_size = ch1WorkBuff.size() * sizeof(float);
        size_t buf_size_rem = buf_size;
        size_t sock_send_size, buf_size_sent = 0;
        do {
          sock_send_size = std::min(buf_size_rem,socketbuffsize);
          char* msg_size = reinterpret_cast<char *>(&sock_send_size);
          LOG(plog::verbose) << "Sending thru socket: " << sock_send_size;
          if (sockets[0] != -1) {
            status = send(sockets[0], msg_size, sizeof(size_t), 0);
            LOG(plog::verbose) << "Tried to send thru socket: " << sock_send_size;
            if (status != sizeof(size_t)) {
              LOG(plog::error) << "Error writing msg_size to socket,"
              #ifdef _WIN32
                                 << " received error: " << WSAGetLastError();
              #else
                                 << " received error: " << std::strerror(errno);
              #endif
              return -1;
            }
            status = send(sockets[0], reinterpret_cast<char *>(ch1WorkBuff.data()+buf_size_sent), sock_send_size, 0);
            if (status < 0 || (size_t)status != sock_send_size) {
              LOG(plog::error) << "Error writing ch1 buffer to socket. "
                                 << "Tried to write " << buf_size << " bytes,"
                                 << "Actually wrote " << status << " bytes.";
              return -1;
            }
          }
          if (sockets[1] != -1) {
            status = send(sockets[1], msg_size, sizeof(size_t), 0);
            if (status != sizeof(size_t)) {
              LOG(plog::error) << "Error writing msg_size to socket,"
              #ifdef _WIN32
                                 << " received error: " << WSAGetLastError();
              #else
                                 << " received error: " << std::strerror(errno);
              #endif
              return -1;
            }
            status = send(sockets[1], reinterpret_cast<char *>(ch2WorkBuff.data()+buf_size_sent), sock_send_size, 0);
            if (status < 0 || (size_t)status != sock_send_size) {
              LOG(plog::error) << "Error writing ch2 buffer to socket. "
                                 << "Tried to write " << buf_size << " bytes,"
                                 << "Actually wrote " << status << " bytes.";
              return -1;
            }
          }
          buf_size_rem = buf_size_rem - sock_send_size;
          buf_size_sent = (buf_size-buf_size_rem)/sizeof(float);
          LOG(plog::verbose) << "Buff size remaining: " << buf_size_rem;
          LOG(plog::verbose) << "Buff size sent: " << buf_size_sent;
        } while(buf_size_rem > 0);
      }
      // repost the buffer if we are not done
      if (bufferCounter < static_cast<int32_t>(nbrBuffers)) {
        if (postBuffer(buff) < 0) {
          LOG(plog::error) << "COULD NOT POST API BUFFER " << std::hex
          << (uint64_t)(buff.get());
          return -1;
        }
      }
    } else {
      // if no socket is available, push it onto dataQ
      dataQ.push(buff);
    }

    if (threadStop) {
      return 0;
    }
    bufferCounter++;
  }
  // stop the card
  retCode = AlazarAbortAsyncRead(boardHandle);
  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
  }
  return 0;
}

int32_t AlazarATS9870::rxThreadRun(void) {
  if (threadRunning) {
    LOG(plog::error) << "RX THREAD ALREADY RUNNING ";
    return -1;
  }

  RETURN_CODE retCode = AlazarBeforeAsyncRead(
      boardHandle, CHANNEL_A | CHANNEL_B, 0, recordLength, recordsPerBuffer,
      recordsPerAcquisition,
      ADMA_NPT | ADMA_EXTERNAL_STARTCAPTURE | ADMA_INTERLEAVE_SAMPLES);
  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
    return -1;
  }

  uint32_t nbrBuffersMaxMin =
      std::min(nbrBuffers, static_cast<uint32_t>(MAX_NUM_BUFFERS));
  nbrBuffersMaxMin =
      std::max(nbrBuffersMaxMin, static_cast<uint32_t>(MIN_NUM_BUFFERS));

  for (uint32_t i = 0; i < nbrBuffersMaxMin; ++i) {
    auto buff = std::make_shared<std::vector<uint8_t>>(bufferLen);
    postBuffer(buff);
  }
  // reset buffer counter
  bufferCounter = 0;

  retCode = AlazarStartCapture(boardHandle);
  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
  }
  int32_t ready = 0;
  uint32_t sleep_time = 50;
  rxThread = std::thread(&AlazarATS9870::rx, this, &ready);
  threadRunning = true;
  while (ready != 1) {
    LOG(plog::info) << "Waiting for Alazar to begin acq";
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
  }
  LOG(plog::info) << "Alazar ready to acq";
  return 0;
}

void AlazarATS9870::rxThreadStop(void) {
  LOG(plog::verbose) << "STOPPING RX THREAD " << rxThread.get_id();
  if (threadRunning) {
    threadStop = true;
    try {
      rxThread.join();
    } catch (std::exception &e) {
      LOG(plog::error) << "Error occured: " << e.what();
    }
    threadRunning = false;

    RETURN_CODE retCode = AlazarAbortAsyncRead(boardHandle);
    if (retCode != ApiSuccess) {
      printError(retCode, __FILE__, __LINE__);
    }

    // Clear the buffers to release the ownership of the shared pointers
    // This will deallocate the memory allocated for the buffers
    // NOTE:
    // Only do this afer the AlazarAbortAsyncRead to make sure that the
    // alazar is done accessing the memory
    std::shared_ptr<std::vector<uint8_t>> buff;
    bufferQ.clear(buff);
    dataQ.clear(buff);
    ownerQ.clear(buff);
  }

  threadStop = false;
}

int32_t AlazarATS9870::postBuffer(shared_ptr<std::vector<uint8_t>> buff) {
  // maintain a copy of the shared pointer in the owner Q to prevent the
  // the pointer from going out of scope in the rx thread
  while (!ownerQ.push(buff))
    ;
  while (!bufferQ.push(buff))
    ;
  RETURN_CODE retCode =
      AlazarPostAsyncBuffer(boardHandle, buff.get()->data(), bufferLen);

  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
    return (-1);
  } else {
    LOG(plog::verbose) << "POSTED BUFFER " << std::hex
                        << (uint64_t)(buff.get());
  }

  return (0);
}

int32_t AlazarATS9870::getBufferSize(void) {

  // need to fit at least one record in a buffer
  if (recordLength * numChannels > MAX_BUFFER_SIZE) {
    LOG(plog::error) << "SINGLE RECORD TOO LARGE FOR THE BUFFER";
    return (-1);
  }

  // Find the factors of the number of round robins.
  // Each buffer will contain an integer number of round robins and the number
  // of round robins will be divided equally amoung all of the buffers.
  std::list<uint32_t> rrFactors;
  for (uint32_t ii = 1; ii <= nbrRoundRobins; ii++) {
    if (nbrRoundRobins % ii == 0) {
      rrFactors.push_front(ii);
    }
  }

  // find the maximum number of rr's that can fit in the buffer
  roundRobinsPerBuffer = 0;
  for (std::list<uint32_t>::iterator rr = rrFactors.begin();
       rr != rrFactors.end(); ++rr) {
    uint64_t bufferSizeTest =
        recordLength * nbrSegments * nbrWaveforms * *rr * numChannels;
    if (bufferSizeTest <= PREF_BUFFER_SIZE) {
      roundRobinsPerBuffer = *rr;
      LOG(plog::info) << "roundRobinsPerBuffer: " << roundRobinsPerBuffer;
      break;
    }
  }

  // if test is 0 then the round robin needs to be divided evenly into an
  // an integer number of buffers
  if (roundRobinsPerBuffer >= 1) {
    // compute the buffer size
    bufferLen = recordLength * nbrSegments * nbrWaveforms *
                roundRobinsPerBuffer * numChannels;
    LOG(plog::info) << "bufferLen: " << bufferLen;

    nbrBuffers = nbrRoundRobins / roundRobinsPerBuffer;
    LOG(plog::info) << "nbrBuffers: " << nbrBuffers;

    recordsPerBuffer = nbrSegments * nbrWaveforms * roundRobinsPerBuffer;
    LOG(plog::info) << "recordsPerBuffer: " << recordsPerBuffer;

    recordsPerAcquisition = recordsPerBuffer * nbrBuffers;
    LOG(plog::info) << "recordsPerAcquisition: " << recordsPerAcquisition;

    partialBuffer = false;
    LOG(plog::info) << "partialBuffer: " << partialBuffer;

    return (0);
  }

  // A round robin must be equally divided into buffers
  // Factor the number of records per round robin.
  // The number of records per buffer will be one of these factors
  uint32_t recordsPerRoundRobin = nbrSegments * nbrWaveforms;
  std::list<uint32_t> recFactors;
  for (uint32_t ii = 1; ii <= recordsPerRoundRobin; ii++) {
    if (recordsPerRoundRobin % ii == 0) {
      recFactors.push_front(ii);
    }
  }

  // Find the number of records that can come closest to the max buffer size
  buffersPerRoundRobin = 0;
  for (std::list<uint32_t>::iterator rec = recFactors.begin();
       rec != recFactors.end(); ++rec) {
    uint32_t bufferSizeTest = recordLength * *rec * numChannels;
    if (bufferSizeTest <= PREF_BUFFER_SIZE) {
      recordsPerBuffer = *rec;
      LOG(plog::info) << "recordsPerBuffer: " << recordsPerBuffer;
      break;
    }
  }

  bufferLen = recordLength * recordsPerBuffer * numChannels;
  LOG(plog::info) << "bufferLen: " << bufferLen;

  recordsPerAcquisition = nbrSegments * nbrWaveforms * nbrRoundRobins;
  LOG(plog::info) << "recordsPerAcquisition: " << recordsPerAcquisition;

  nbrBuffers = recordsPerAcquisition / recordsPerBuffer;
  LOG(plog::info) << "nbrBuffers: " << nbrBuffers;

  partialBuffer = true;
  LOG(plog::info) << "partialBuffer: " << partialBuffer;

  buffersPerRoundRobin = nbrBuffers / nbrRoundRobins;
  LOG(plog::info) << "buffersPerRoundRobin: " << buffersPerRoundRobin;

  if (buffersPerRoundRobin * recordsPerBuffer * recordLength >
      MAX_BUFFER_SIZE) {
    LOG(plog::error) << " Exeeded MAX_BUFFER_SIZE";
    return (-1);
  }

  return (0);
}

int32_t AlazarATS9870::processBuffer(
    std::shared_ptr<std::vector<uint8_t>> buffPtr, float *ch1, float *ch2) {
  if (partialBuffer) {
    return processPartialBuffer(buffPtr, ch1, ch2);
  } else {
    return processCompleteBuffer(buffPtr, ch1, ch2);
  }
}

int32_t
AlazarATS9870::processCompleteBuffer(std::shared_ptr<std::vector<uint8_t>> buffPtr,
                             float *ch1, float *ch2) {

  // accumulate the average in the application buffer which needs to
  // be cleared to start
  memset(ch1, 0, sizeof(float) * samplesPerAcquisition);
  memset(ch2, 0, sizeof(float) * samplesPerAcquisition);

  // the raw pointer makes the code more readable
  uint8_t *buff = static_cast<uint8_t *>(buffPtr.get()->data());

  if (averager) {
    // copy and sum along the 2nd and 4th dimension
    uint32_t ni = recordLength;
    uint32_t nj = nbrWaveforms;
    uint32_t nk = nbrSegments;
    uint32_t nl = roundRobinsPerBuffer;

    for (uint32_t l = 0; l < nl; l++) {
      for (uint32_t k = 0; k < nk; k++) {
        for (uint32_t j = 0; j < nj; j++) {
          for (uint32_t i = 0; i < ni; i++) {
            // ch1 and ch2 samples are interleaved for faster transfer times
            ch1[i + k*ni] +=
                counts2Volts * (buff[2*i + j*2*ni + k*2*ni*nj + l*2*ni*nj*nk] -
                                128) -
                channelOffset;
            ch2[i + k*ni] +=
                counts2Volts * (buff[2*i + 1 + j*2*ni + k*2*ni*nj + l*2*ni*nj*nk] -
                                128) -
                channelOffset;
          }
        }
      }
    }

    float denom = nj * nl;
    for (uint32_t i = 0; i < ni * nk; i++) {
      ch1[i] /= denom;
      ch2[i] /= denom;
    }
  } else { // digitizer mode
    for (uint32_t i = 0; i < bufferLen / 2; i++) {
      ch1[i] = counts2Volts * (buff[2 * i] - 128) - channelOffset;
      ch2[i] = counts2Volts * (buff[2 * i + 1] - 128) - channelOffset;
    }
  }

  return 1;
}

int32_t AlazarATS9870::processPartialBuffer(
    std::shared_ptr<std::vector<uint8_t>> buffPtr, float *ch1, float *ch2) {
  uint32_t partialIndex = bufferCounter % buffersPerRoundRobin;
  LOG(plog::verbose) << "PARTIAL INDEX " << partialIndex;

  // the raw pointer makes the code more readable
  uint8_t *buff = static_cast<uint8_t *>(buffPtr.get()->data());

  if (averager) {
     float *pCh1Work =  ch1WorkBuff.data();
     float *pCh2Work =  ch2WorkBuff.data();

    // process the buff into the work buffer and if it is the last buffer
    // in the round robin, run the averager
    float *pCh1 = (float *)( pCh1Work + bufferLen * partialIndex / 2);
    float *pCh2 = (float *)( pCh1Work + bufferLen * partialIndex / 2);

    for (uint32_t i = 0; i < bufferLen / 2; i++) {
      pCh1[i] = counts2Volts * (buff[2 * i] - 128) - channelOffset;
      pCh2[i] = counts2Volts * (buff[2 * i + 1] - 128) - channelOffset;
    }

    if (partialIndex == buffersPerRoundRobin - 1) {
      // accumulate the average in the application buffer which needs to
      // be cleared to start
      memset(ch1, 0, sizeof(float) * samplesPerAcquisition);
      memset(ch2, 0, sizeof(float) * samplesPerAcquisition);

      uint32_t ni = recordLength;
      uint32_t nj = nbrWaveforms;
      uint32_t nk = nbrSegments;

      // run the averager
      for (uint32_t k = 0; k < nk; k++) {
        for (uint32_t j = 0; j < nj; j++) {
          for (uint32_t i = 0; i < ni; i++) {
            // ch1 and ch2 samples are interleaved for faster transfer times
            ch1[i + k * ni] += pCh1Work[i + j * ni + k * ni * nj];
            ch2[i + k * ni] += pCh2Work[i + j * ni + k * ni * nj];
          }
        }
      }

      float denom = nj;
      for (uint32_t i = 0; i < ni * nk; i++) {
        ch1[i] /= denom;
        ch2[i] /= denom;
      }
    }
  } else {
    float *pCh1 = (float *)(ch1 + bufferLen * partialIndex / 2);
    float *pCh2 = (float *)(ch2 + bufferLen * partialIndex / 2);

    for (uint32_t i = 0; i < bufferLen / 2; i++) {
      pCh1[i] = counts2Volts * (buff[2 * i] - 128) - channelOffset;
      pCh2[i] = counts2Volts * (buff[2 * i + 1] - 128) - channelOffset;
    }
  }

  if (partialIndex == buffersPerRoundRobin - 1) {
    return 1;
  } else {
    return 0;
  }
}

void AlazarATS9870::printError(RETURN_CODE code, std::string file,
                               int32_t line) {

  LOG(plog::error) << "File: " << file << " Line: " << line
                     << " ERROR: " << std::to_string(code) << " "
                     << AlazarErrorToText(code);
}

int32_t AlazarATS9870::sysInfo() {
  // log the sdk RevisionNumber
  uint8_t major, minor, rev;
  RETURN_CODE retCode = AlazarGetSDKVersion(&major, &minor, &rev);
  if (retCode != ApiSuccess) {
    printError(retCode, __FILE__, __LINE__);
  } else {
    LOG(plog::info) << "ATS SDK Rev " << to_string(major) << "."
                      << to_string(minor) << "." << to_string(rev);
  }

  // see how many boards are installed
  // todo - need to think aobout error handling here - create a info method
  uint32_t systemCount = AlazarNumOfSystems();
  LOG(plog::info) << "System count    = " << to_string(systemCount);

  if (systemCount > 0) {
    for (uint32_t systemId = 1; systemId <= systemCount; systemId++) {
      if (!DisplaySystemInfo(systemId))
        return 1;
    }
  }

  return 0;
}

// uint32_t systemCount() {
//   return AlazarNumOfSystems();
// }

// uint32_t boardCount(uint32_t systemId) {
//   return AlazarBoardsInSystemBySystemID(systemId);
// }

// std::string boardInfo(uint32_t systemId, uint32_t boardId) {
//   std::ostringstream ss;
//   uint32_t serialNumber;
//   HANDLE handle = AlazarGetBoardBySystemID(systemId, boardId);
//   RETURN_CODE retCode = AlazarQueryCapability(handle, GET_SERIAL_NUMBER, 0, &serialNumber);

//   if (retCode != ApiSuccess) {
//     LOG(plog::error) << "AlazarQueryCapability failed -- "
//                        << AlazarErrorToText(retCode);
//     ss << "Alazar ATS9870 UnknownSerial";
//   } else {
//     ss << "Alazar ATS9870 " << to_string(serialNumber);
//   }
//   return ss.str();
// }

//---------------------------------------------------------------------------
//
// Function    :  DisplaySystemInfo
//
// Description :  Display information about this board system
//
//---------------------------------------------------------------------------

int32_t AlazarATS9870::DisplaySystemInfo(uint32_t systemId) {
  uint32_t boardCount = AlazarBoardsInSystemBySystemID(systemId);
  if (boardCount == 0) {
    LOG(plog::error) << "No boards found in system";
    return -1;
  }

  HANDLE handle = AlazarGetSystemHandle(systemId);
  if (handle == NULL) {
    LOG(plog::error) << "AlazarGetSystemHandle system failed";
    return -1;
  }

  uint32_t boardType = AlazarGetBoardKind(handle);
  if (boardType == ATS_NONE || boardType >= ATS_LAST) {
    LOG(plog::error) << "Unknown board type " << boardType;
    return -1;
  }

  uint8_t driverMajor, driverMinor, driverRev;
  RETURN_CODE retCode =
      AlazarGetDriverVersion(&driverMajor, &driverMinor, &driverRev);
  if (retCode != ApiSuccess) {
    LOG(plog::error) << "AlazarGetDriverVersion failed "
                       << AlazarErrorToText(retCode);
    return -1;
  }

  LOG(plog::info) << "System ID       = " << systemId;
  LOG(plog::info) << "Board type      = " << BoardTypeToText(boardType);
  LOG(plog::info) << "Board count     = " << boardCount;
  LOG(plog::info) << "Driver version  = " << to_string(driverMajor) << "."
                    << to_string(driverMinor) << "." << to_string(driverRev);

  // Display informataion about each board in this board system

  for (uint32_t boardId = 1; boardId <= boardCount; boardId++) {
    if (!DisplayBoardInfo(systemId, boardId))
      return -1;
  }

  return 0;
}

//---------------------------------------------------------------------------
//
// Function    :  DisplayBoardInfo
//
// Description :  Display information about a board
//
//---------------------------------------------------------------------------

int32_t AlazarATS9870::DisplayBoardInfo(uint32_t systemId, uint32_t boardId) {
  RETURN_CODE retCode;

  HANDLE handle = AlazarGetBoardBySystemID(systemId, boardId);
  if (handle == NULL) {
    LOG(plog::error) << "Open systemId " << systemId << " boardId " << boardId
                       << " failed";
    return -1;
  }

  U32 samplesPerChannel;
  BYTE bitsPerSample;
  retCode = AlazarGetChannelInfo(handle, &samplesPerChannel, &bitsPerSample);
  if (retCode != ApiSuccess) {
    LOG(plog::error) << "AlazarGetChannelInfo failed -- "
                       << AlazarErrorToText(retCode);
    return -1;
  }

  U32 aspocType;
  retCode = AlazarQueryCapability(handle, ASOPC_TYPE, 0, &aspocType);
  if (retCode != ApiSuccess) {
    LOG(plog::error) << "AlazarQueryCapability failed -- "
                       << AlazarErrorToText(retCode);
    return -1;
  }

  BYTE cpldMajor;
  BYTE cpldMinor;
  retCode = AlazarGetCPLDVersion(handle, &cpldMajor, &cpldMinor);
  if (retCode != ApiSuccess) {
    LOG(plog::error) << "AlazarGetCPLDVersion failed -- "
                       << AlazarErrorToText(retCode);
    return -1;
  }

  U32 serialNumber;
  retCode = AlazarQueryCapability(handle, GET_SERIAL_NUMBER, 0, &serialNumber);
  if (retCode != ApiSuccess) {
    LOG(plog::error) << "AlazarQueryCapability failed -- "
                       << AlazarErrorToText(retCode);
    return -1;
  }

  U32 latestCalDate;
  retCode =
      AlazarQueryCapability(handle, GET_LATEST_CAL_DATE, 0, &latestCalDate);
  if (retCode != ApiSuccess) {
    LOG(plog::error) << "Error: AlazarQueryCapability failed -- "
                       << AlazarErrorToText(retCode);
    return -1;
  }

  LOG(plog::info) << "System ID                 = " << systemId;
  LOG(plog::info) << "Board ID                  = " << boardId;
  LOG(plog::info) << "Serial number             = " << setw(6)
                    << serialNumber;
  LOG(plog::info) << "Bits per sample           = "
                    << to_string(bitsPerSample);
  LOG(plog::info) << "Max samples per channel   = " << samplesPerChannel;
  LOG(plog::info) << "CPLD version              = " << to_string(cpldMajor)
                    << "." << to_string(cpldMinor);
  LOG(plog::info) << "FPGA version              = "
                    << ((aspocType >> 16) & 0xff) << "."
                    << ((aspocType >> 24) & 0xf);
  LOG(plog::info) << "ASoPC signature           = " << setw(8) << hex
                    << aspocType;
  LOG(plog::info) << "Latest calibration date   = " << latestCalDate;

  if (IsPcieDevice(handle)) {
    // Display PCI Express link information

    U32 linkSpeed;
    retCode = AlazarQueryCapability(handle, GET_PCIE_LINK_SPEED, 0, &linkSpeed);
    if (retCode != ApiSuccess) {
      LOG(plog::error) << "AlazarQueryCapability failed -- "
                         << AlazarErrorToText(retCode);
      return -1;
    }

    U32 linkWidth;
    retCode = AlazarQueryCapability(handle, GET_PCIE_LINK_WIDTH, 0, &linkWidth);
    if (retCode != ApiSuccess) {
      LOG(plog::error) << "Error: AlazarQueryCapability failed -- "
                         << AlazarErrorToText(retCode);
      return -1;
    }

    LOG(plog::info) << "PCIe link speed           = " << 2.5 * linkSpeed
                      << " Gbps";
    LOG(plog::info) << "PCIe link width           = " << linkWidth
                      << " lanes";
  }

  // Toggling the LED on the PCIe/PCIe mounting bracket of the board

  int cycleCount = 2;
  uint32_t cyclePeriod_ms = 200;

  if (!FlashLed(handle, cycleCount, cyclePeriod_ms))
    return -1;

  return 0;
}

//---------------------------------------------------------------------------
//
// Function    :  IsPcieDevice
//
// Description :  Return 0 if board has PCIe host bus interface
//
//---------------------------------------------------------------------------

bool AlazarATS9870::IsPcieDevice(HANDLE handle) {
  uint32_t boardType = AlazarGetBoardKind(handle);
  if (boardType >= ATS9462)
    return true;
  else
    return false;
}

//---------------------------------------------------------------------------
//
// Function    :  FlashLed
//
// Description :  Flash LED on board's PCI/PCIe mounting bracket
//
//---------------------------------------------------------------------------

int32_t AlazarATS9870::FlashLed(HANDLE handle, int32_t cycleCount,
                                uint32_t cyclePeriod_ms) {
  for (int cycle = 0; cycle < cycleCount; cycle++) {
    LOG(plog::verbose) << "Flashing LED...";

    const int phaseCount = 2;
    uint32_t sleepPeriod_ms = cyclePeriod_ms / phaseCount;

    for (int phase = 0; phase < phaseCount; phase++) {
      uint32_t state = (phase == 0) ? LED_ON : LED_OFF;
      RETURN_CODE retCode = AlazarSetLED(handle, state);
      if (retCode != ApiSuccess)
        printError(retCode, __FILE__, __LINE__);

      if (sleepPeriod_ms > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepPeriod_ms));
    }
  }

  return 0;
}

//---------------------------------------------------------------------------
//
// Function    :  BoardTypeToText
//
// Description :  Convert board type Id to text
//
//---------------------------------------------------------------------------

string AlazarATS9870::BoardTypeToText(int boardType) {
  string pszName;

  switch (boardType) {
  case ATS850:
    pszName = "ATS850";
    break;
  case ATS310:
    pszName = "ATS310";
    break;
  case ATS330:
    pszName = "ATS330";
    break;
  case ATS855:
    pszName = "ATS855";
    break;
  case ATS315:
    pszName = "ATS315";
    break;
  case ATS335:
    pszName = "ATS335";
    break;
  case ATS460:
    pszName = "ATS460";
    break;
  case ATS860:
    pszName = "ATS860";
    break;
  case ATS660:
    pszName = "ATS660";
    break;
  case ATS9461:
    pszName = "ATS9461";
    break;
  case ATS9462:
    pszName = "ATS9462";
    break;
  case ATS9850:
    pszName = "ATS9850";
    break;
  case ATS9870:
    pszName = "ATS9870";
    break;
  case ATS9310:
    pszName = "ATS9310";
    break;
  case ATS9325:
    pszName = "ATS9325";
    break;
  case ATS9350:
    pszName = "ATS9350";
    break;
  case ATS9351:
    pszName = "ATS9351";
    break;
  case ATS9410:
    pszName = "ATS9410";
    break;
  case ATS9440:
    pszName = "ATS9440";
    break;
  case ATS_NONE:
  default:
    pszName = "?";
  }

  return pszName;
}

//---------------------------------------------------------------------------
//
// Function    :  force_trigger()
//
// Description :  force a software trigger
//
//---------------------------------------------------------------------------
int32_t AlazarATS9870::force_trigger( void )
{

    RETURN_CODE retCode = AlazarForceTrigger(boardHandle);
    if (retCode != ApiSuccess)
    {
        printError(retCode,__FILE__,__LINE__);
    }
    return(retCode);
}
