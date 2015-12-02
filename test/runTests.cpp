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

  int count=5;

  while( count-- > 0)
  {
      usleep(500000);
      FILE_LOG(logINFO) << "MAIN" ;

  }
  
  stop();
    
}