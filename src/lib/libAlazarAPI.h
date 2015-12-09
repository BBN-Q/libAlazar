#ifndef LIBALAZARAPI_H_
#define LIBALAZARAPI_H_

#include <stdint.h>

#include "libAlazarConfig.h"
//#include "libAlazar.h"

#ifdef __cplusplus
extern "C" {
#endif


int32_t connect( const char* );
int32_t disconnect(void);
int32_t setAll(void);
int32_t acquire(void);
int32_t wait_for_acquisition(void);
int32_t stop();
int32_t flash_led(int32_t numTimes, float period);
int32_t transfer_waveform(void);

#ifdef __cplusplus
}
#endif


#endif