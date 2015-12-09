#ifndef LIBALAZARCONFIG_H_
#define LIBALAZARCONFIG_H_

#include <stdint.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NUM_BUFFERS 32


typedef struct ConfigData
{            
    char *      acquireMode;
    uint32_t    address;
    char *      bandwidth;
    char *      clockType;
    double      delay;
    bool        enabled;
    char *      label;
    uint32_t    nbrRoundRobins;
    uint32_t    nbrSegments;
    uint32_t    recordLength;
    double      samplingRate;
    char *      triggerCoupling;
    double      triggerLevel;
    char *      triggerSlope;
    char *      triggerSource;
    char *      verticalCoupling;
    double      verticalOffset;
    double      verticalScale;
}ConfigData_t;

#ifdef __cplusplus
}
#endif


#endif