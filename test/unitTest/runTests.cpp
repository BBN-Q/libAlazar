#include <unistd.h>
#include <map>

#include "libAlazarAPI.h"
#include "logger.h"
#include "libAlazarConfig.h"

                       
int main( int argc, char *argv[])
{
    ConfigData_t config;

    connectBoard(NULL);
    config.address = (const char*)"ADDR1";
    setAll( config );
    acquire();

    int count=100;

   while( count-- > 0)
   {
       FILE_LOG(logDEBUG4) << "MAIN" ;
       wait_for_acquisition();
       usleep(10000);

   }
  
   stop();
   disconnect();
    
    
}

