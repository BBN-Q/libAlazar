#include <cstdlib>
#include <cstdio>
#include <stdio.h>
#include <string>
#include <thread>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "logger.h"
#include "libAlazar.h"

using namespace std;

AlazarATS9870 *board1=NULL;

#ifdef __cplusplus
extern "C"{
#endif    

int32_t connect( const char* logFile )
{    
    FILE *pFile = stdout;
    
    if (logFile)
    {
        pFile = fopen(logFile, "a"); 
    }
    
    Output2FILE::Stream() = pFile;       
    
    board1 = new AlazarATS9870;    

    return(0);

}

int32_t setAll(void)
{
    if( !board1 )
    {
        return(-1);
    }
    return 0;
}

int32_t acquire(void)
{
    
    if( !board1 )
    {
        return(-1);
    }
    
    if( board1->threadRunning )
    {
        return(-1);
    }
    
    board1->rxThreadRun();   
 
    return 0;
}

int32_t wait_for_acquisition(void)
{
    if( !board1 )
    {
        return(-1);
    }
    
    return 0;
}

int32_t stop()
{
    if( !board1 )
    {
        return(-1);
    }
    
    board1->rxThreadStop();
    

    return 0;
}

int32_t disconnect(void)
{
    if( !board1 )
    {
        return(-1);
    }
    
    if( board1->threadRunning)
    {
        stop();
    }
    
    delete board1;
    board1 = NULL;
    return 0;
    
}

int32_t transfer_waveform(void)
{
    return 0;
}

int32_t flash_led(int32_t numTimes, float period)
{
    FILE_LOG(logDEBUG4) << "Flashing LED ... ";
    return 0;
}


#ifdef __cplusplus
}
#endif