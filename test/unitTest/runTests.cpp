//#include <cstdlib>
//#include <string>
//#include <thread>

#include <unistd.h>

#include "libAlazarAPI.h"
#include "logger.h"
#include "libAlazarConfig.h"

                       
int main( void)
{
    ConfigData_t config;
    
    connect(NULL);
    config.address = (const char*)"ADDR1";
    setAll( config );
    acquire();

    int count=100;

   while( count-- > 0)
   {
       FILE_LOG(logINFO) << "MAIN" ;
       wait_for_acquisition();
       usleep(10000);

   }
  
   stop();
   disconnect();
    
    
}

