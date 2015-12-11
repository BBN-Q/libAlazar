#ifndef LIBALAZARAPI_H_
#define LIBALAZARAPI_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ConfigData
{            
    const char *    acquireMode;
    const char*     address;
    const char *    bandwidth;
    const char *    clockType;
    double          delay;
    bool            enabled;
    char *          label;
    uint32_t        nbrRoundRobins;
    uint32_t        nbrSegments;
    uint32_t        recordLength;
    double          samplingRate;
    const char *    triggerCoupling;
    double          triggerLevel;
    const char *    triggerSlope;
    const char *    triggerSource;
    const char *    verticalCoupling;
    double          verticalOffset;
    double          verticalScale;
}ConfigData_t;



int32_t connect( const char* );
int32_t disconnect(void);
int32_t setAll(ConfigData_t config);
int32_t acquire(void);
int32_t wait_for_acquisition(void);
int32_t stop();
int32_t flash_led(int32_t numTimes, float period);
int32_t transfer_waveform(void);

#ifdef __cplusplus
}
#endif


#endif