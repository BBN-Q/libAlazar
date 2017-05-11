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

#ifndef LIBALAZARAPI_H_
#define LIBALAZARAPI_H_

#include <stdint.h>

#ifdef _WIN32
#define APIEXPORT __declspec(dllexport)
#else
#define APIEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ConfigData {
  const char *acquireMode;
  const char *bandwidth;
  const char *clockType; // todo - parameter currently not used
  double delay;
  bool enabled;
  const char *label;
  uint32_t recordLength;
  uint32_t nbrSegments;
  uint32_t nbrWaveforms;
  uint32_t nbrRoundRobins;
  double samplingRate;
  const char *triggerCoupling;
  double triggerLevel;
  const char *triggerSlope;
  const char *triggerSource;
  const char *verticalCoupling;
  double verticalOffset;
  double verticalScale;
} ConfigData_t;

typedef struct AcquisitionParams {
  uint32_t samplesPerAcquisition;
  uint32_t numberAcquisitions;
} AcquisitionParams_t;

APIEXPORT int32_t connectBoard(uint32_t boardID, const char *);
APIEXPORT int32_t disconnect(uint32_t boardID);
APIEXPORT int32_t setAll(uint32_t boardId, const ConfigData_t *config,
                      AcquisitionParams_t *acqParams);

APIEXPORT int32_t acquire(uint32_t boardId);
APIEXPORT int32_t wait_for_acquisition(uint32_t boardID, float *ch1, float *ch2);
APIEXPORT int32_t stop(uint32_t boardID);
APIEXPORT int32_t flash_led(int32_t numTimes, float period);
APIEXPORT int32_t force_trigger( uint32_t boardID );
APIEXPORT int32_t register_socket(uint32_t boardID, uint32_t channel, int32_t socket);
APIEXPORT int32_t unregister_sockets(uint32_t boardID);

#ifdef __cplusplus
}
#endif

#endif
