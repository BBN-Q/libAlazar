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

#ifndef LIBALAZAR_H_
#define LIBALAZAR_H_

#include <array>
#include <atomic>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <stdint.h>
#include <string>
#include <thread>

#include "AlazarApi.h"
#include "AlazarCmd.h"
#include "AlazarError.h"
#include "alazarBuff.h"
#include "libAlazarAPI.h"

#define MAX_NUM_BUFFERS 32
#define MIN_NUM_BUFFERS 2
#define MAX_BUFFER_SIZE 256000000 // 256M
#define PREF_BUFFER_SIZE 4000000 // 4M (suggestion from Alazar manual for DMA transfers)
#define SOCKET_TX_MAX 219264


uint32_t systemCount();
uint32_t boardCount(uint32_t systemId);
std::string boardInfo(uint32_t systemId, uint32_t boardId);


class AlazarATS9870 {

public:
  const uint32_t numChannels = 2;

  std::atomic<bool> threadStop;
  std::atomic<bool> threadRunning;

  // These ptr queues are used to manage the DMA buffer pointers passed to the
  // alazar dll.
  // The library uses the bufferQ ptr to retreive data from the alazar in the
  // receive thread.  When the data is ready the lib places the ptr into the
  // dataQ.  The wait_for_acquisition API call polls the dataQ, and then
  // processes
  // the data into the application supplied buffers.
  // The ownerQ is used to maintain a copy of the shared ptr beacuse a ptr
  // can be popped off the bufferQ in the receive thread and go out of
  // scope resulting in the allocated buffer memory being freed too soon.
  AlazarBufferQ<std::shared_ptr<std::vector<uint8_t>>> bufferQ;
  AlazarBufferQ<std::shared_ptr<std::vector<uint8_t>>> dataQ;
  AlazarBufferQ<std::shared_ptr<std::vector<uint8_t>>> ownerQ;

  std::atomic<int32_t> bufferCounter;

  // socket for sending data back to a listening client
  int32_t sockets[2] = {-1, -1};

  static std::map<RETURN_CODE, std::string> errorMap;

  // this working buffer is used for the partial buffer logic when a round
  // robin is distributed over multiple buffers

  std::vector<float> ch1WorkBuff;
  std::vector<float> ch2WorkBuff;

  bool averager;

  uint32_t bufferLen;
  bool partialBuffer;
  uint32_t roundRobinsPerBuffer;
  uint32_t buffersPerRoundRobin;
  float counts2Volts;
  float channelOffset;

  uint32_t recordLength;
  uint32_t nbrSegments;
  uint32_t nbrWaveforms;
  uint32_t nbrRoundRobins;
  uint32_t nbrBuffers;

  uint32_t samplesPerAcquisition;
  uint32_t sampleRate;

  AlazarATS9870();
  ~AlazarATS9870();
  int32_t sysInfo(void);
  int32_t rxThreadRun(void);
  void rxThreadStop(void);

  int32_t postBuffer(std::shared_ptr<std::vector<uint8_t>>);
  void printError(RETURN_CODE code, std::string file, int32_t line);
  
  int32_t Connect(uint32_t systemId, uint32_t boardId);
  int32_t SetMode(const char *acquireMode);
  int32_t SetSampleRate(uint32_t samplingRate);
  int32_t ConfigureVertical(float verticalScale, float verticalOffset,
                                         const char *verticalCoupling);
  int32_t SetBandwidth(const char *bandwidth);
  int32_t ConfigureTrigger(float triggerLevel, const char *triggerSource,
                           const char *triggerSlope, const char *triggerCoupling, float delay);
  int32_t ConfigureAcquisition(uint32_t recordLength, uint32_t nbrSegments, 
                               uint32_t nbrWaveforms, uint32_t nbrRoundRobins,
                               AcquisitionParams_t &acqParams);

  int32_t ConfigureBoard(uint32_t systemId, uint32_t boardId,
                         const ConfigData_t &config,
                         AcquisitionParams_t &acqParams);

  int32_t processBuffer(std::shared_ptr<std::vector<uint8_t>> buff,
                        float *ch1, float *ch2);
  int32_t processCompleteBuffer(std::shared_ptr<std::vector<uint8_t>> buff,
                                float *ch1, float *ch2);
  int32_t processPartialBuffer(std::shared_ptr<std::vector<uint8_t>> buff,
                               float *ch1, float *ch2);
  int32_t force_trigger( void );

protected:
  ConfigData_t config;
  std::thread rxThread;
  HANDLE boardHandle;

  int32_t DisplaySystemInfo(uint32_t);
  int32_t DisplayBoardInfo(uint32_t systemId, uint32_t boardId);
  bool IsPcieDevice(HANDLE handle);
  int32_t FlashLed(HANDLE handle, int32_t cycleCount, uint32_t cyclePeriod_ms);

  std::string BoardTypeToText(int boardType);
  int32_t rx(int32_t *ready);
  int32_t getBufferSize(void);

  // map mV input scale to RangeId
  float channelScale;
  std::map<uint32_t, uint32_t> rangeIdMap = {
      {40, INPUT_RANGE_PM_40_MV},   {100, INPUT_RANGE_PM_100_MV},
      {200, INPUT_RANGE_PM_200_MV}, {400, INPUT_RANGE_PM_400_MV},
      {1000, INPUT_RANGE_PM_1_V},   {2000, INPUT_RANGE_PM_2_V},
      {4000, INPUT_RANGE_PM_4_V},
  };

  // map coupling input
  std::map<std::string, uint32_t> couplingMap = {
      {"AC", 1}, {"DC", 2},
  };

  // map bandwidth string to control value
  std::map<std::string, uint32_t> bandwidthMap = {
      {"Full", 0}, {"20MHz", 1},
  };

  std::map<std::string, uint32_t> triggerSourceMap = {
      {"A", 0}, {"B", 1}, {"Ext", 2},
  };

  std::map<std::string, uint32_t> triggerSlopeMap = {
      {"rising", 1}, {"falling", 2},
  };

  uint32_t bufferSize;
  const uint32_t maxOnboardMemory = 0x10000000; // 256MB

  uint32_t recordsPerBuffer;
  uint32_t recordsPerAcquisition;
  size_t  socketbuffsize;

  std::map<std::string, bool> modeMap = {
      {"digitizer", false}, {"averager", true},
  };
};

#endif
