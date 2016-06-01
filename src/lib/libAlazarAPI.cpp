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

#define MAX_NUM_BOARDS 2
AlazarATS9870 boards[MAX_NUM_BOARDS];

#ifdef __cplusplus
extern "C"{
#endif

int32_t connectBoard( uint32_t boardId, const char* logFile )
{
    FILE *pFile = stdout;

    if( boardId > 0 && boardId <= MAX_NUM_BOARDS)
    {
        AlazarATS9870 &board = boards[boardId-1];
        board.sysInfo();
    }
    else
    {
        FILE_LOG(logERROR) << "Invalid board address "<< boardId;
        return(-1);
    }


    if (logFile)
    {
        pFile = fopen(logFile, "w");
    }

    Output2FILE::Stream() = pFile;
    FILE_LOG(logINFO) << "libAlazar Rev "<<std::string(VERSION);

    return(0);

}

int32_t setAll(uint32_t boardId, const ConfigData_t *config,
    AcquisitionParams_t *acqParams)
{
    AlazarATS9870 &board = boards[boardId-1];

    if( config == nullptr || acqParams == nullptr)
    {
        FILE_LOG(logERROR) << "COULD NOT SET CONFIGURATION ";
        return(-1);
    }

    const ConfigData_t &confRef = static_cast<const ConfigData_t&>(*config);
    AcquisitionParams_t &acqRef = static_cast<AcquisitionParams_t&>(*acqParams);

    int32_t ret = board.ConfigureBoard(1, boardId, confRef, acqRef);

    return ret;
}

int32_t acquire(uint32_t boardId)
{
    AlazarATS9870 &board = boards[boardId-1];
    int32_t ret=0;

    if( board.threadRunning )
    {
        return(-1);
    }

    ret = board.rxThreadRun();

    return ret;
}

int32_t wait_for_acquisition(uint32_t boardId,float *ch1, float *ch2)
{
    AlazarATS9870 &board = boards[boardId-1];

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
    shared_ptr<std::vector<uint8_t>> buff;
    if(!board.dataQ.pop(buff))
    {
        return 0;
    }

    FILE_LOG(logDEBUG4) << "API POPPING DATA " << std::hex << (uint64_t)(buff.get());

    // if there are multiple buffers per roundrobin the partial index logic
    // is used to process the data from the individual buffers into one
    // application channel buffer
    int32_t ret=0;
    if (board.partialBuffer)
    {
        ret = board.processPartialBuffer(buff, ch1, ch2);
    }
    else
    {
        ret = board.processBuffer(buff, ch1, ch2);
    }

    if( board.postBuffer(buff) >= 0 )
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

int32_t stop(uint32_t boardId)
{
    AlazarATS9870 &board = boards[boardId-1];

    board.rxThreadStop();

    return 0;
}

int32_t disconnect(uint32_t boardId)
{
    AlazarATS9870 &board = boards[boardId-1];

    if( board.threadRunning)
    {
        stop(boardId);
    }

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
