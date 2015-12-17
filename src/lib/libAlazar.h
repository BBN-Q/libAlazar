#ifndef LIBALAZAR_H_
#define LIBALAZAR_H_

#include <stdint.h>
#include <string>
#include <iostream>
#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>


#include "libAlazarConfig.h"
#include "libAlazarAPI.h"

class AlazarATS9870
{
    
    public:
        
        std::atomic<bool> threadStop;
        std::atomic<bool> threadRunning;
            
        boost::lockfree::spsc_queue<int8_t*,  boost::lockfree::capacity<MAX_NUM_BUFFERS>> bufferQ;
        boost::lockfree::spsc_queue<int8_t*, boost::lockfree::capacity<MAX_NUM_BUFFERS>> dataQ;
        
        std::atomic<int32_t> bufferCounter;

        AlazarATS9870();
        ~AlazarATS9870();
        void rxThreadRun( void );
        void rxThreadStop( void );
        
        void postBuffer( int8_t *buff);
        
    protected:
        
        ConfigData_t config;
        
        
        std::thread rxThread;
        
        int32_t bufferLen;
        

        int32_t rx( void );       
        
    
};


#endif