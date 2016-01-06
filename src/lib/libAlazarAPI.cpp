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
#include "libAlazarConfig.h"
#include "version.h"

using namespace std;

AlazarATS9870 *board1=NULL;

#ifdef __cplusplus
extern "C"{
#endif    

int32_t connectBoard( const char* logFile )
{    
    FILE *pFile = stdout;
    
    if (logFile)
    {
        pFile = fopen(logFile, "a"); 
    }
    
    Output2FILE::Stream() = pFile;    
    FILE_LOG(logINFO) << "libAlazar Rev "<<std::string(VERSION);
   
    
    board1 = new AlazarATS9870;   
    board1->sysInfo(); 

    return(0);

}

int32_t setAll(ConfigData_t config)
{
    if( !board1 )
    {
        return(-1);
    }
    
    FILE_LOG(logDEBUG4) << "CONFIG address " << config.address ;
    
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
    
    //wait for a buffer to be ready
    int8_t *buff;
    if(!board1->dataQ.pop(buff))
    {
        return 0;
    }
    FILE_LOG(logDEBUG4) << "API POPPING DATA " << std::hex << (int64_t)buff ;
    board1->bufferCounter++;
    
    //do stuff with it
    
    
    //return the buffer
    FILE_LOG(logDEBUG4) << "API POSTED BUFFER " << std::hex << (int64_t)buff ;
    board1->postBuffer(buff);




    
    
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