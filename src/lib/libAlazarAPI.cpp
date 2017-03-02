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

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <string>
#include <thread>
#include <time.h>

#include "libAlazar.h"
#include "logger.h"
#include "version.h"

using namespace std;

#define MAX_NUM_BOARDS 2
AlazarATS9870 boards[MAX_NUM_BOARDS];

#ifdef __cplusplus
extern "C" {
#endif

int32_t connectBoard(uint32_t boardId, const char *logFile) {
  FILE *pFile = stdout;

  if (boardId > 0 && boardId <= MAX_NUM_BOARDS) {
    AlazarATS9870 &board = boards[boardId - 1];
    board.sysInfo();
  } else {
    FILE_LOG(logERROR) << "Invalid board address " << boardId;
    return (-1);
  }

  if (logFile) {
    pFile = fopen(logFile, "w");
  }

  Output2FILE::Stream() = pFile;
  FILE_LOG(logINFO) << "libAlazar Rev " << std::string(VERSION);

  return (0);
}

int32_t setAll(uint32_t boardId, const ConfigData_t *config,
               AcquisitionParams_t *acqParams) {
  AlazarATS9870 &board = boards[boardId - 1];

  if (config == nullptr || acqParams == nullptr) {
    FILE_LOG(logERROR) << "COULD NOT SET CONFIGURATION ";
    return (-1);
  }

  const ConfigData_t &confRef = static_cast<const ConfigData_t &>(*config);
  AcquisitionParams_t &acqRef = static_cast<AcquisitionParams_t &>(*acqParams);

  int32_t ret = board.ConfigureBoard(1, boardId, confRef, acqRef);

  return ret;
}

int32_t acquire(uint32_t boardId) {
  AlazarATS9870 &board = boards[boardId - 1];
  int32_t ret = 0;

  if (board.threadRunning) {
    return (-1);
  }

  ret = board.rxThreadRun();

  return ret;
}

// returns 0 (no new data) or 1 (new data)
int32_t wait_for_acquisition(uint32_t boardId, float *ch1, float *ch2) {
  AlazarATS9870 &board = boards[boardId - 1];

  if (board.sockets[0] != -1 || board.sockets[1] != -1) {
      FILE_LOG(logERROR) << "wait_for_acquisition should not be used with the socket API.";
      return -1;
  }

  if (ch1 == NULL) {
    FILE_LOG(logERROR) << "NULL Pointer to Ch1";
    return (-1);
  }

  if (ch2 == NULL) {
    FILE_LOG(logERROR) << "NULL Pointer to Ch2";
    return (-1);
  }

  // wait for a buffer to be ready
  shared_ptr<std::vector<uint8_t>> buff;
  if (!board.dataQ.pop(buff)) {
    return 0;
  }

  FILE_LOG(logDEBUG4) << "API POPPING DATA " << std::hex
                      << (uint64_t)(buff.get());

  // if there are multiple buffers per roundrobin the partial index logic
  // is used to process the data from the individual buffers into one
  // application channel buffer
  int32_t ret = board.processBuffer(buff, ch1, ch2);

  if (board.postBuffer(buff) >= 0) {
    FILE_LOG(logDEBUG4) << "API POSTED BUFFER " << std::hex
                        << (uint64_t)(buff.get());
  } else {
    FILE_LOG(logERROR) << "COULD NOT POST API BUFFER " << std::hex
                       << (uint64_t)(buff.get());
    return (-1);
  }

  return ret;
}

int32_t stop(uint32_t boardId) {
  AlazarATS9870 &board = boards[boardId - 1];

  board.rxThreadStop();

  return 0;
}

int32_t disconnect(uint32_t boardId) {
  AlazarATS9870 &board = boards[boardId - 1];

  if (board.threadRunning) {
    stop(boardId);
  }

  return 0;
}

int32_t force_trigger(uint32_t boardId)
{
    AlazarATS9870 &board = boards[boardId-1];
    int32_t retCode = board.force_trigger();
    return retCode;

}
int32_t flashLED(uint32_t boardId)
{
  FILE_LOG(logDEBUG4) << "Flashing LED ... ";
  return 0;
}

int32_t register_socket(uint32_t boardId, uint32_t channel, int32_t socket) {
    AlazarATS9870 &board = boards[boardId - 1];
    if (channel >= board.numChannels) {
        FILE_LOG(logERROR) << "Invalid channel";
        return -1;
    }
    board.sockets[channel] = socket;
    return 0;
}

#ifdef __cplusplus
}
#endif
