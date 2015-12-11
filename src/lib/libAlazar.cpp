#include <cstdlib>
#include <string>
#include <thread>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "logger.h"
#include "libAlazar.h"
#include "libAlazarApi.h"
#include "AlazarApi.h"

using namespace std;

AlazarATS9870::AlazarATS9870()
{
    FILE_LOG(logDEBUG4) << "Constructing ... ";
    threadStop = false;
    threadRunning = false;
    bufferLen = (int32_t)1e6;
}

AlazarATS9870::~AlazarATS9870()
{
    FILE_LOG(logDEBUG4) << "Destructing ...";
}

int32_t AlazarATS9870::rx( void)
{
    
    while( 1)
    {
        int8_t *buff=NULL;
        while(!bufferQ.pop(buff))
        {
            if ( threadStop )
            {
                return 0;
            }
        }
        FILE_LOG(logDEBUG4) << "RX POPPED BUFFER " << std::hex << (int64_t)buff ;
        AlazarWaitAsyncBufferComplete(0,buff,0);
        while(!dataQ.push(buff))
        {
            if ( threadStop )
            {
                return 0;
            }
            
        }
        FILE_LOG(logDEBUG4) << "RX POSTING DATA " << std::hex << (int64_t)buff;


        if( threadStop )
        {
            return 0;
        }
        
    }
    
    
    return 0;

}

void AlazarATS9870::rxThreadRun( void )
{
    
    int8_t *buff=NULL;
    for (int i = 0; i != MAX_NUM_BUFFERS; ++i) 
    {
        buff= (int8_t *)malloc(bufferLen);
        postBuffer(buff);

    }  
    
    rxThread = std::thread( &AlazarATS9870::rx, this );
    FILE_LOG(logDEBUG4) << "STARTING RX THREAD" ;
    threadRunning = true;

}

void AlazarATS9870::rxThreadStop( void )
{
    if( threadRunning)
    {
        threadStop = true;
        rxThread.join();
        threadRunning = false;
    }
    FILE_LOG(logDEBUG4) << "STOPPING RX THREAD" ;
    threadStop = false;
}
void AlazarATS9870::postBuffer( int8_t *buff)
{
    while (!bufferQ.push(buff));
    AlazarPostAsyncBuffer(0,buff,bufferLen);
}



