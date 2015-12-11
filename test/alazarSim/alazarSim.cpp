#include <cstdlib>
#include <string>
#include <thread>
#include <unistd.h>
#include <boost/lockfree/spsc_queue.hpp>

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
