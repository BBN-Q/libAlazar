#ifndef LIBALAZAR_H_
#define LIBALAZAR_H_

#include <stdint.h>
#include <string>
#include <iostream>
#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>
#include <map>
#include <thread>


#include "libAlazarConfig.h"
#include "libAlazarAPI.h"
#include "AlazarApi.h"
#include "AlazarError.h"
#include "AlazarCmd.h"

class AlazarATS9870
{

    public:

        std::atomic<bool> threadStop;
        std::atomic<bool> threadRunning;

        boost::lockfree::spsc_queue<int8_t*,  boost::lockfree::capacity<MAX_NUM_BUFFERS>> bufferQ;
        boost::lockfree::spsc_queue<int8_t*, boost::lockfree::capacity<MAX_NUM_BUFFERS>> dataQ;

        std::atomic<int32_t> bufferCounter;

        static std::map<RETURN_CODE,std::string> errorMap;

        AlazarATS9870();
        ~AlazarATS9870();
        int32_t sysInfo( void );
        void rxThreadRun( void );
        void rxThreadStop( void );

        void postBuffer( int8_t *buff);
        void printError(RETURN_CODE code, std::string file, int32_t line );


    protected:

        ConfigData_t config;
        std::thread rxThread;
        int32_t bufferLen;
        int32_t DisplaySystemInfo( uint32_t);
        int32_t DisplayBoardInfo(uint32_t systemId, uint32_t boardId);
        bool IsPcieDevice(HANDLE handle);
        int32_t FlashLed(HANDLE handle, int32_t cycleCount, uint32_t cyclePeriod_ms);

        std::string BoardTypeToText(int boardType);
        int32_t rx( void );



};


#endif
