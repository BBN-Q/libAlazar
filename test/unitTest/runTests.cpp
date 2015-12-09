//#include <cstdlib>
//#include <string>
//#include <thread>

#include <unistd.h>

#include "libAlazarAPI.h"
#include "logger.h"

int main( void)
{
  connect(NULL);

  acquire();

  int count=20000000;

  while( count-- > 0)
  {
      FILE_LOG(logINFO) << "MAIN" ;
      wait_for_acquisition();
      usleep(5000);

  }
  
  stop();
  disconnect();
    
    
}

