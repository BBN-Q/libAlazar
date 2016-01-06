#include <cstdlib>
#include <string>
#include <thread>
#include <unistd.h>
#include <boost/lockfree/spsc_queue.hpp>
#include <map>

#include "logger.h"
#include "AlazarApi.h"
#include "libAlazarConfig.h"

static boost::lockfree::spsc_queue<int8_t*,
	boost::lockfree::capacity<MAX_NUM_BUFFERS>> bufferQ;

RETURN_CODE AlazarPostAsyncBuffer (
	HANDLE  hDevice,
	void   *pBuffer, 
	U32     uBufferLength_bytes
	)
{    
    while(!bufferQ.push((int8_t*)pBuffer));
    FILE_LOG(logDEBUG4) << "SIM POSTING BUFFER " << std::hex 
		<< (int64_t)pBuffer ;

    return ApiSuccess;
}


RETURN_CODE  AlazarWaitAsyncBufferComplete(HANDLE hDevice, void *pBuffer,
                                                 U32 uTimeout_ms)
{
    int8_t *temp;
    while(!bufferQ.pop(temp));
    if( temp == pBuffer)
    {
        FILE_LOG(logDEBUG4) << "SIM SENDING DATA " << std::hex << (int64_t)pBuffer ;
        return ApiSuccess;

    }
    else
    {
        FILE_LOG(logERROR) << "BUFFER IS OUT OF ORDER" ;
        return ApiInvalidBuffer;
    }
}    
                                        
RETURN_CODE AlazarGetSDKVersion (
      uint8_t *MajorNumber,
      uint8_t *MinorNumber,
      uint8_t *RevisionNumber
)
{
	*MajorNumber = 6;
	*MinorNumber = 0;
	*RevisionNumber = 3;
	
	return ApiSuccess;
}

uint32_t AlazarNumOfSystems()
{
	return 1;
}


/*
NOTE:  The list of enums used for the return codes in the API header file from
the windows SDK is not the same as what is used in the linux SDK.  BUT the ATSApi.dll
is built with the complete set of error codes that matches the linux SDK API 
header.
BOTTOM LINE: the most recent Windows SDK provided does not match the DLL build
todo - since the complete list of enums is available in the Linux API header file
I could add thos to this map
See issue #8
*/
std::map<RETURN_CODE,std::string> errorMap =
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

const char* AlazarErrorToText(RETURN_CODE retCode)
{
	if( retCode < ApiSuccess || retCode > ApiLastError)
	{
		return "Unknown";
	}
	else
	{
		return errorMap[retCode].c_str();
	}
	

}

U32 AlazarBoardsInSystemBySystemID(U32 sid)
{
	return 1;
}

HANDLE AlazarGetBoardBySystemID(U32 sid, U32 brdNum)
{
	return NULL;
}

U32 AlazarGetBoardKind(HANDLE h)
{
	return ATS9850;
}

RETURN_CODE AlazarGetCPLDVersion(HANDLE h, U8 *Major, U8 *Minor)
{
	*Major = 1;
	*Minor = 0;
	return ApiSuccess;
}

RETURN_CODE AlazarGetChannelInfo (
      HANDLE BoardHandle,
      U32 *MemorySizeInSamples,
      U8 *BitsPerSample
)
{
	*MemorySizeInSamples = 256000000;
	*BitsPerSample = 8;
	return ApiSuccess;
}
RETURN_CODE AlazarGetDriverVersion (
      U8 *MajorNumber,
      U8 *MinorNumber,
      U8 *RevisionNumber
)
{
	*MajorNumber = 5;
	*MinorNumber = 10;
	*RevisionNumber = 6;
	return ApiSuccess;
	
}

uint32_t dummy;
HANDLE AlazarGetSystemHandle (
      U32 SystemId
)
{
	return (HANDLE)&dummy;
}

RETURN_CODE AlazarQueryCapability (
      HANDLE BoardHandle,
      U32 Capability,
      U32 Reserved,
      U32 *Value
)
{
	
	switch( Capability )
	{
		default:
		*Value = 0;
		break;
		
	}
	
	
	
	return ApiSuccess;
}

RETURN_CODE AlazarSetLED (
      HANDLE BoardHandle,
	  U32 LedOn 
)
{
	return ApiSuccess;
}

