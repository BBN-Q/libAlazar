#include <cstdlib>
#include <string>
#include <thread>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "logger.h"
#include "libAlazar.h"

using namespace std;

AlazarATS9870::AlazarATS9870()
{
    FILE_LOG(logDEBUG4) << "Constructing ... ";
    threadStop = false;
    threadRunning = false;
}

AlazarATS9870::~AlazarATS9870()
{
    FILE_LOG(logDEBUG4) << "Destructing ...";
}

int32_t AlazarATS9870::rx( void)
{
    int32_t count=100;
    
    //todo - this only currently works for a single instance of this class
    //       more work needed to make this generic for multiple boards in a 
    //       system
    
    while( count-- > 0)
    {
        FILE_LOG(logDEBUG4) << "FOO" ;
        usleep(500000);
        if( threadStop )
        {
            return 0;
        }

    }
    return 0;

}
void AlazarATS9870::rxThreadRun( void )
{
    FILE_LOG(logDEBUG4) << "STARTING RX THREAD" ;
    threadRunning = true;
    rxThread = std::thread( &AlazarATS9870::rx, this );
}

void AlazarATS9870::rxThreadStop( void )
{
    FILE_LOG(logDEBUG4) << "STOPPING RX THREAD" ;
    if( threadRunning)
    {
        threadStop = true;
        rxThread.join();
        threadRunning = false;
    }
    threadStop = false;
}

