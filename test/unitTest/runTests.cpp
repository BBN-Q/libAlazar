#include <unistd.h>
#include <map>
#include <thread>
#include <string>
#include <iostream>
#include <chrono>

#include "libAlazarAPI.h"
#include "logger.h"
#include "libAlazarConfig.h"

void waitForQuit( void )
{
    std::string line;

    std::string strq ("q");
    while(1)
    {
        getline(std::cin,line);
        std::cout << line << std::endl;
        if( !line.compare(strq))
        {
            printf("Exiting ...\n");
            exit(0);
        }

    }
    return;
}


int main( int argc, char *argv[])
{
    //todo - make parameters user configurable
    const ConfigData_t config =
    {
        "averager",  //acquire mode - "digitizer" or "averager"
        "Full",       //bandwidth - "Full" or "20MHz"
        "ref", //todo - clockType, parameter currently not used
        0.0, // trigger delay in seconds
        true, //instrumrnt enabled
        "myAlazar", //instrument label
        4096, //segmentLength - must be greater than 256 and a multiple of 16
        1, // number of segments
        4, // number of waveforms
        1, // number of round robins
        500e6, // sample rate
        "DC", // trigger coupling - "AC" or "DC"
        1000, // trigger threshold in mV
        "rising", // trigger edge slope - "rising" or "falling"
        "Ext", //trigger source -  only "Ext" supported
        "AC", // channel coupling - "AC" or "DC"
        0.0, // channel offset
        4.0, // channel scale
        4096*2, // max buffer size
    };

    AcquisitionParams_t acqParams;

    //this is a hack because because MSYS2 does not handle
    std::thread quit(waitForQuit);
    quit.detach();


    connectBoard(NULL);
    if( setAll( 1,1,&config, &acqParams ) < 0 )
    {
        exit(-1);
    }

    //allocate channel data memory
    float *ch1 = new float[acqParams.samplesPerAcquisition];
    float *ch2 = new float[acqParams.samplesPerAcquisition];

    //open a couple of files for storing the data
    FILE *f1 = fopen("ch1.dat","wb");
    FILE *f2 = fopen("ch2.dat","wb");

    acquire();

    #if 1
    uint32_t count=0;
    while( count < acqParams.numberAcquistions )
    {
        //printf("rr %d count %d\n",config.nbrRoundRobins,count);
        if( wait_for_acquisition(ch1, ch2) )
        {
            count++;
            fwrite(ch1,sizeof(float),acqParams.samplesPerAcquisition,f1);
            fwrite(ch2,sizeof(float),acqParams.samplesPerAcquisition,f2);
        }
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    #endif
    stop();
    disconnect();

    delete[] ch1;
    delete[] ch2;
    fclose(f1);
    fclose(f2);


}
