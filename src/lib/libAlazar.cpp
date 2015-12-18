#include <cstdlib>
#include <string>
#include <thread>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "logger.h"
#include "libAlazar.h"
#include "libAlazarApi.h"

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
        //todo check error code
        RETURN_CODE retCode = AlazarWaitAsyncBufferComplete(0,buff,0);
        if( retCode != ApiSuccess)
        {            
            printError(retCode,__FILE__,__LINE__);
        }

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
    RETURN_CODE retCode = AlazarPostAsyncBuffer(0,buff,bufferLen);
    if( retCode != ApiSuccess)
    {
        printError(retCode,__FILE__,__LINE__);
    }

}

std::map<RETURN_CODE,std::string> AlazarATS9870::errorMap =
{
    {ApiSuccess,"ApiSuccess"},
    {ApiFailed,"ApiFailed"},
    {ApiAccessDenied,"ApiAccessDenied"},
    {ApiDmaChannelUnavailable,"ApiDmaChannelUnavailable"},
    {ApiDmaChannelInvalid,"ApiDmaChannelInvalid"},
    {ApiDmaChannelTypeError,"ApiDmaChannelTypeError"},
    {ApiDmaInProgress,"ApiDmaInProgress"},
    {ApiDmaDone,"ApiDmaDone"},
    {ApiDmaPaused,"ApiDmaPaused"},
    {ApiDmaNotPaused,"ApiDmaNotPaused"},
    {ApiDmaCommandInvalid,"ApiDmaCommandInvalid"},
    {ApiDmaManReady,"ApiDmaManReady"},
    {ApiDmaManNotReady,"ApiDmaManNotReady"},
    {ApiDmaInvalidChannelPriority,"ApiDmaInvalidChannelPriority"},
    {ApiDmaManCorrupted,"ApiDmaManCorrupted"},
    {ApiDmaInvalidElementIndex,"ApiDmaInvalidElementIndex"},
    {ApiDmaNoMoreElements,"ApiDmaNoMoreElements"},
    {ApiDmaSglInvalid,"ApiDmaSglInvalid"},
    {ApiDmaSglQueueFull,"ApiDmaSglQueueFull"},
    {ApiNullParam,"ApiNullParam"},
    {ApiInvalidBusIndex,"ApiInvalidBusIndex"},
    {ApiUnsupportedFunction,"ApiUnsupportedFunction"},
    {ApiInvalidPciSpace,"ApiInvalidPciSpace"},
    {ApiInvalidIopSpace,"ApiInvalidIopSpace"},
    {ApiInvalidSize,"ApiInvalidSize"},
    {ApiInvalidAddress,"ApiInvalidAddress"},
    {ApiInvalidAccessType,"ApiInvalidAccessType"},
    {ApiInvalidIndex,"ApiInvalidIndex"},
    {ApiMuNotReady,"ApiMuNotReady"},
    {ApiMuFifoEmpty,"ApiMuFifoEmpty"},
    {ApiMuFifoFull,"ApiMuFifoFull"},
    {ApiInvalidRegister,"ApiInvalidRegister"},
    {ApiDoorbellClearFailed,"ApiDoorbellClearFailed"},
    {ApiInvalidUserPin,"ApiInvalidUserPin"},
    {ApiInvalidUserState,"ApiInvalidUserState"},
    {ApiEepromNotPresent,"ApiEepromNotPresent"},
    {ApiEepromTypeNotSupported,"ApiEepromTypeNotSupported"},
    {ApiEepromBlank,"ApiEepromBlank"},
    {ApiConfigAccessFailed,"ApiConfigAccessFailed"},
    {ApiInvalidDeviceInfo,"ApiInvalidDeviceInfo"},
    {ApiNoActiveDriver,"ApiNoActiveDriver"},
    {ApiInsufficientResources,"ApiInsufficientResources"},
    {ApiObjectAlreadyAllocated,"ApiObjectAlreadyAllocated"},
    {ApiAlreadyInitialized,"ApiAlreadyInitialized"},
    {ApiNotInitialized,"ApiNotInitialized"},
    {ApiBadConfigRegEndianMode,"ApiBadConfigRegEndianMode"},
    {ApiInvalidPowerState,"ApiInvalidPowerState"},
    {ApiPowerDown,"ApiPowerDown"},
    {ApiFlybyNotSupported,"ApiFlybyNotSupported"},
    {ApiNotSupportThisChannel,"ApiNotSupportThisChannel"},
    {ApiNoAction,"ApiNoAction"},
    {ApiHSNotSupported,"ApiHSNotSupported"},
    {ApiVPDNotSupported,"ApiVPDNotSupported"},
    {ApiVpdNotEnabled,"ApiVpdNotEnabled"},
    {ApiNoMoreCap,"ApiNoMoreCap"},
    {ApiInvalidOffset,"ApiInvalidOffset"},
    {ApiBadPinDirection,"ApiBadPinDirection"},
    {ApiPciTimeout,"ApiPciTimeout"},
    {ApiDmaChannelClosed,"ApiDmaChannelClosed"},
    {ApiDmaChannelError,"ApiDmaChannelError"},
    {ApiInvalidHandle,"ApiInvalidHandle"},
    {ApiBufferNotReady,"ApiBufferNotReady"},
    {ApiInvalidData,"ApiInvalidData"},
    {ApiDoNothing,"ApiDoNothing"},
    {ApiDmaSglBuildFailed,"ApiDmaSglBuildFailed"},
    {ApiPMNotSupported,"ApiPMNotSupported"},
    {ApiInvalidDriverVersion,"ApiInvalidDriverVersion"},
    {ApiWaitTimeout,"ApiWaitTimeout"},
    {ApiWaitCanceled,"ApiWaitCanceled"},
    {ApiBufferTooSmall,"ApiBufferTooSmall"},
    {ApiBufferOverflow,"ApiBufferOverflow"},
    {ApiInvalidBuffer,"ApiInvalidBuffer"},
    {ApiInvalidRecordsPerBuffer,"ApiInvalidRecordsPerBuffer"},
    {ApiDmaPending,"ApiDmaPending"},
    {ApiLockAndProbePagesFailed,"ApiLockAndProbePagesFailed"},
    {ApiWaitAbandoned,"ApiWaitAbandoned"},
    {ApiWaitFailed,"ApiWaitFailed"},
    {ApiTransferComplete,"ApiTransferComplete"},
    {ApiPllNotLocked,"ApiPllNotLocked"},
    {ApiNotSupportedInDualChannelMode,"ApiNotSupportedInDualChannelMode"},    
    {ApiLastError ,"ApiLastError"},

};

void AlazarATS9870::printError(RETURN_CODE code, std::string file, int32_t line )
{
    if( code < ApiSuccess || code > ApiLastError)
    {
        FILE_LOG(logERROR) << "File: " << file << " Line: "<< line << " ERROR: " << std::to_string(code) << " " << "Invalid API Error Code" ;
    }
    else
    {
        FILE_LOG(logERROR) << "File: " << file << " Line: "<< line << " ERROR: " << std::to_string(code) << " " << errorMap[code] ;
        
    }
    
}

