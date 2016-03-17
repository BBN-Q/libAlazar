#include <cstdlib>
#include <iostream>
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


    board1 = new AlazarATS9870();
    board1->sysInfo();

    return(0);

}

int32_t setAll(uint32_t systemId, uint32_t boardId, const ConfigData_t *config,
    AcquisitionParams_t *acqParams)
{
    
    if( config == nullptr || acqParams == nullptr)
    {
        FILE_LOG(logERROR) << "COULD NOT SET CONFIGURATION ";
        return(-1);
    }
    
    
    
    if( !board1 )
    {
        return(-1);
    }

    const ConfigData_t &confRef = static_cast<const ConfigData_t&>(*config);
    AcquisitionParams_t &acqRef = static_cast<AcquisitionParams_t&>(*acqParams);

    int32_t ret = board1->ConfigureBoard(systemId, boardId, confRef, acqRef);

    return ret;
}

int32_t acquire(void)
{
    int32_t ret=0;

    if( !board1 )
    {
        return(-1);
    }

    if( board1->threadRunning )
    {
        return(-1);
    }

    ret = board1->rxThreadRun();

    return ret;
}

int32_t wait_for_acquisition(float *ch1, float *ch2)
{
    if( !board1 )
    {
        return(-1);
    }

    if( ch1 == NULL)
    {
        FILE_LOG(logERROR) << "NULL Pointer to Ch1";
        return(-1);
    }

    if( ch2 == NULL)
    {
        FILE_LOG(logERROR) << "NULL Pointer to Ch2";
        return(-1);
    }

    //wait for a buffer to be ready
    shared_ptr<std::vector<int8_t>> buff;
    if(!board1->dataQ.pop(buff))
    {
        return 0;
    }

    FILE_LOG(logDEBUG4) << "API POPPING DATA " << std::hex << (uint64_t)(buff.get());

    // if there are multiple buffers per roundrobin the partial index logic
    // is used to process the data from the individual buffers into one
    // application channel buffer
    int32_t ret=0;
    if (board1->partialBuffer)
    {
        ret = board1->processPartialBuffer(buff, ch1, ch2);
    }
    else
    {
        ret = board1->processBuffer(buff, ch1, ch2);
    }

    if( board1->postBuffer(buff) >= 0 )
    {
        FILE_LOG(logDEBUG4) << "API POSTED BUFFER " << std::hex << (uint64_t)(buff.get()) ;
    }
    else
    {
        FILE_LOG(logERROR) << "COULD NOT POST API BUFFER " << std::hex << (uint64_t)(buff.get()) ;
        return(-1);
    }

    return(ret);

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
