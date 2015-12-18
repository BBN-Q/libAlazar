//#include <cstdlib>
//#include <string>
//#include <thread>
//#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <map>

#include "AlazarError.h"
#include "libAlazar.h"

                       
int main( int argc, char *argv[])
{
    if( argc < 2)
    {
        printf("USAGE: %s <numerical error code>\n",argv[0]);
        exit(-1);
    }
    
    AlazarATS9870 board1;    

    
    std::cout << "ERROR TEST ..." << std::endl;
    board1.printError((RETURN_CODE)std::stoi(argv[1]),__FILE__,__LINE__);
    
    return(0);
}

