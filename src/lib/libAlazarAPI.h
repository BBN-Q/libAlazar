#ifndef LIBALAZARAPI_H_
#define LIBALAZARAPI_H_

#include <stdint.h>

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

int32_t connectBoard(uint32_t boardID, const char *);
int32_t disconnect(uint32_t boardID);
int32_t setAll(uint32_t boardId, const ConfigData_t *config,
               AcquisitionParams_t *acqParams);

int32_t acquire(uint32_t boardId);
int32_t wait_for_acquisition(uint32_t boardID, float *ch1, float *ch2);
int32_t stop(uint32_t boardID);
int32_t flash_led(int32_t numTimes, float period);
int32_t force_trigger( uint32_t boardID );

#ifdef __cplusplus
}
#endif

#endif
