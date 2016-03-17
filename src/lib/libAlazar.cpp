#include <cstdlib>
#include <string>
#include <thread>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <chrono>
#include <list>
#include <algorithm>


#include "logger.h"
#include "libAlazar.h"
#include "libAlazarAPI.h"

using namespace std;

AlazarATS9870::AlazarATS9870(): threadStop(false), threadRunning(false)
{
    FILE_LOG(logDEBUG4) << "Constructing ... ";
}

AlazarATS9870::~AlazarATS9870()
{
    FILE_LOG(logDEBUG4) << "Destructing ...";

    RETURN_CODE retCode = AlazarCloseAUTODma(boardHandle);
    if (retCode != ApiSuccess)
	{
        printError(retCode,__FILE__,__LINE__);
	}

    AlazarClose(boardHandle);
}

int32_t AlazarATS9870::ConfigureBoard(uint32_t systemId, uint32_t boardId,
    const ConfigData_t &config, AcquisitionParams_t &acqParams)
{

    boardHandle = AlazarGetBoardBySystemID(systemId, boardId);
    if (boardHandle == NULL)
    {
        FILE_LOG(logERROR) << "Open systemId " << systemId << " boardId "<<boardId<<" failed";
        return -1;
    }

    if( config.bufferSize > MAX_BUFFER_SIZE )
    {
        FILE_LOG(logERROR) << "MAX_BUFFER_SIZE Exceeded: " << config.bufferSize << " < " << MAX_BUFFER_SIZE;
        return -1;

    }

    //set averager mode or digitizer mode
    const char* acquireModeKey= config.acquireMode;
    if( modeMap.find(acquireModeKey) == modeMap.end() )
    {
        FILE_LOG(logERROR) << "Invalid Mode: " <<  acquireModeKey;
        return(-1);
    }
    averager = modeMap[config.acquireMode];

    // set the sample rate parameters:
    // SampleRateId is set to 1e9 and there is an external ref clock configured
    // so the sample rate is 1e9/decimation; decimation factor has to be 1,2,4
    // or any multiple of 10
    uint32_t decimation = 1000000000/static_cast<uint32_t>(config.samplingRate);
    FILE_LOG(logINFO) << "Decimation " << decimation;
    if(decimation != 1 && decimation != 2 && decimation != 4)
    {
        if(decimation%10 != 0)
        {
            FILE_LOG(logERROR) << "Decimation is not a Mulitple of 2,4,or 10 ";
            return(-1);
        }
    }

	RETURN_CODE retCode =
        AlazarSetCaptureClock(
        	boardHandle,			  // HANDLE -- board handle
        	EXTERNAL_CLOCK_10MHz_REF, // U32 -- clock source id
        	1000000000,		          // U32 -- sample rate id - 1e9
        	CLOCK_EDGE_RISING,	      // U32 -- clock edge id
        	decimation			      // U32 -- clock decimation
        	);
	if (retCode != ApiSuccess)
	{
        printError(retCode,__FILE__,__LINE__);
        return -1;
	}

    //set up the channel parameters for channel A
    channelScale = config.verticalScale;
    channelOffset = config.verticalOffset;
    counts2Volts = 2*channelScale/256.0;

    uint32_t rangeIDKey= static_cast<uint32_t>(config.verticalScale*1000);
    if( rangeIdMap.find(rangeIDKey) == rangeIdMap.end() )
    {
        FILE_LOG(logERROR) << "Invalid Channel Scale: " <<  rangeIDKey;
        return(-1);
    }

    const char* couplingKey= config.verticalCoupling;
    if( couplingMap.find(couplingKey) == couplingMap.end() )
    {
        FILE_LOG(logERROR) << "Invalid Channel Coupling: " <<  couplingKey;
        return(-1);
    }

    FILE_LOG(logINFO) << "Input Range: " << channelScale << " ID: " << rangeIdMap[rangeIDKey];
    FILE_LOG(logINFO) << "Counts2Volts: " << counts2Volts;

    retCode =
        AlazarInputControl(
            boardHandle,			// HANDLE -- board handle
            CHANNEL_A,				// U8 -- input channel
            couplingMap[couplingKey],	// U32 -- input coupling id
            //TODO verify values for vertical scale
            rangeIdMap[rangeIDKey],		// U32 -- input range id
            IMPEDANCE_50_OHM		// U32 -- input impedance id
            );
    if (retCode != ApiSuccess)
    {
        printError(retCode,__FILE__,__LINE__);
        return -1;
    }

    const char* bamdwithKey= config.bandwidth;
    if( bamdwithMap.find(bamdwithKey) == bamdwithMap.end() )
    {
        FILE_LOG(logERROR) << "Invalid Mode: " <<  bamdwithKey;
        return(-1);
    }
    retCode =
        AlazarSetBWLimit(
            boardHandle,			// HANDLE -- board handle
            CHANNEL_A,				// U8 -- channel identifier
            bamdwithMap[config.bandwidth]	// U32 -- 0 = disable, 1 = enable
            );
    if (retCode != ApiSuccess)
    {
        printError(retCode,__FILE__,__LINE__);
        return -1;
    }

    //set up the channel parameters for channel B
    retCode =
        AlazarInputControl(
            boardHandle,			// HANDLE -- board handle
            CHANNEL_B,				// U8 -- input channel
            couplingMap[config.verticalCoupling],			// U32 -- input coupling id
            rangeIdMap[static_cast<uint32_t>(config.verticalScale*1000)],		// U32 -- input range id
            IMPEDANCE_50_OHM		// U32 -- input impedance id
            );
    if (retCode != ApiSuccess)
    {
        printError(retCode,__FILE__,__LINE__);
        return -1;
    }

    retCode =
        AlazarSetBWLimit(
            boardHandle,			// HANDLE -- board handle
            CHANNEL_B,				// U8 -- channel identifier
            bamdwithMap[config.bandwidth]	// U32 -- 0 = disable, 1 = enable
            );
    if (retCode != ApiSuccess)
    {
        printError(retCode,__FILE__,__LINE__);
        return -1;
    }

    // Select trigger inputs and levels as required
    //trigLevelCode = uint8(128 + 127*(trigSettings.triggerLevel/1000/trigChannelRange));
    //uint32_t = conf->triggerLevel
    uint32_t trigChannelRange = 5;
    uint32_t trigLevelCode = static_cast<uint32_t>(128 + 127*(config.triggerLevel/1000/trigChannelRange));
    FILE_LOG(logINFO) << "Trigger Level Code " << trigLevelCode;

    const char* triggerSourceKey= config.triggerSource;
    if( triggerSourceMap.find(triggerSourceKey) == triggerSourceMap.end() )
    {
        FILE_LOG(logERROR) << "Invalid Trigger Source ID: " <<  triggerSourceKey;
        return(-1);
    }

    const char* triggerSlopeMapKey= config.triggerSlope;
    if( triggerSlopeMap.find(triggerSlopeMapKey) == triggerSlopeMap.end() )
    {
        FILE_LOG(logERROR) << "Invalid Trigger Coupling: " <<  triggerSlopeMapKey;
        return(-1);
    }

    retCode =
        AlazarSetTriggerOperation(
            boardHandle,			// HANDLE -- board handle
            TRIG_ENGINE_OP_J,		// U32 -- trigger operation
            TRIG_ENGINE_J,			// U32 -- trigger engine id
            triggerSourceMap[config.triggerSource],			// U32 -- trigger source id
            triggerSlopeMap[config.triggerSlope],	// U32 -- trigger slope id
            trigLevelCode,					// U32 -- trigger level from 0 (-range) to 255 (+range)
            TRIG_ENGINE_K,			// U32 -- trigger engine id
            TRIG_DISABLE,			// U32 -- trigger source id for engine K
            TRIGGER_SLOPE_POSITIVE,	// U32 -- trigger slope id
            128						// U32 -- trigger level from 0 (-range) to 255 (+range)
            );
    if (retCode != ApiSuccess)
    {
        printError(retCode,__FILE__,__LINE__);
        return -1;
    }

    //set up external triggerCoupling
    //TODO - add channel based triggering
    couplingKey= config.triggerCoupling;
    if( couplingMap.find(couplingKey) == couplingMap.end() )
    {
        FILE_LOG(logERROR) << "Invalid Trigger Coupling: " <<  couplingKey;
        return(-1);
    }

    retCode =
		AlazarSetExternalTrigger(
			boardHandle,			// HANDLE -- board handle
			couplingMap[config.triggerCoupling],			// U32 -- external trigger coupling id
			ETR_5V					// U32 -- external trigger range id
			);
    if (retCode != ApiSuccess)
    {
        printError(retCode,__FILE__,__LINE__);
        return -1;
    }

    //set the trigger delay in samples
    uint32_t trigDelayPts = config.samplingRate * config.delay;
    FILE_LOG(logINFO) << "Trigger Delay " << trigDelayPts;
    retCode = AlazarSetTriggerDelay(boardHandle, trigDelayPts);
	if (retCode != ApiSuccess)
	{
        printError(retCode,__FILE__,__LINE__);
        return -1;
	}

    //set timeout to 0 - don't time out waiting for a trigger
    retCode =
		AlazarSetTriggerTimeOut(
			boardHandle,			// HANDLE -- board handle
			0	// U32 -- timeout_sec / 10.e-6 (0 means wait forever)
			);
	if (retCode != ApiSuccess)
	{
        printError(retCode,__FILE__,__LINE__);
        return -1;
	}


    recordLength = config.recordLength;

    if( recordLength < 256)
    {
        FILE_LOG(logERROR) << "recordLength less than 256" ;
        return -1;
    }

    if( recordLength % 64 != 0)
    {
        FILE_LOG(logERROR) << "recordLength is not aligned on a 64 sample boundary" ;
        return -1;
    }

    FILE_LOG(logINFO) << "recordLength: " << recordLength;
    retCode = AlazarSetRecordSize(boardHandle,0,recordLength);
    if( retCode != ApiSuccess)
    {
        printError(retCode,__FILE__,__LINE__);
    }


    nbrSegments = config.nbrSegments;
    nbrWaveforms = config.nbrWaveforms;
    nbrRoundRobins = config.nbrRoundRobins;
    bufferSize = config.bufferSize;
    FILE_LOG(logINFO) << "allocated bufferSize: " << bufferSize;



    //compute records per buffer and records per acquisition
    if( getBufferSize() < 0 )
    {
        return(-1);
    }

    //The application uses this info allocate its channel data buffers
    if( !partialBuffer )
    {
        if( averager )
        {
            acqParams.samplesPerAcquisition = recordLength*nbrSegments;
        }
        else
        {
            acqParams.samplesPerAcquisition = recordLength*nbrSegments*nbrWaveforms*roundRobinsPerBuffer;
        }
        acqParams.numberAcquistions = nbrBuffers;
    }
    else
    {
        if( averager )
        {
            acqParams.samplesPerAcquisition = recordLength*nbrSegments;
        }
        else
        {
            acqParams.samplesPerAcquisition = recordLength*nbrSegments*nbrWaveforms;
        }
        acqParams.numberAcquistions = nbrBuffers/buffersPerRoundRobin;

    }
    
    samplesPerAcquisition = acqParams.samplesPerAcquisition;
    FILE_LOG(logINFO) << "samplesPerAcquisition: " << samplesPerAcquisition;
    FILE_LOG(logINFO) << "numberAcquistions: " << acqParams.numberAcquistions;

    return 0;
}


int32_t AlazarATS9870::rx( void)
{
    RETURN_CODE retCode;
    uint32_t count=0;
    FILE_LOG(logDEBUG4) << "STARTING RX THREAD" ;

    while( 1)
    {
        std::shared_ptr<std::vector<int8_t>> buff;
        while(!bufferQ.pop(buff))
        {
            if ( threadStop )
            {
                return 0;
            }
        }

        while(1)
        {

            if( threadStop)
            {
                return 0;
            }
            retCode = AlazarWaitAsyncBufferComplete(boardHandle,buff.get()->data(),1000);//1 sec timeout
            if( retCode == ApiWaitTimeout)
            {
                continue;
            }
            else if( retCode == ApiSuccess)
            {
                FILE_LOG(logDEBUG4) << "GOT BUFFER " << count++ ;
                break;
            }
            else
            {
                printError(retCode,__FILE__,__LINE__);
                return -1;
            }
        }

        while(!dataQ.push(buff))
        {
            if ( threadStop )
            {
                return 0;
            }
        }

        if( threadStop )
        {
            return 0;
        }
    }
    return 0;
}

int32_t AlazarATS9870::rxThreadRun( void )
{

    RETURN_CODE retCode = AlazarBeforeAsyncRead(
        boardHandle,
        CHANNEL_A | CHANNEL_B,
        0,
        recordLength,
        recordsPerBuffer,
        recordsPerAcquisition,
        //0x7FFFFFFF,
        ADMA_NPT | ADMA_EXTERNAL_STARTCAPTURE | ADMA_INTERLEAVE_SAMPLES
    );
    if( retCode != ApiSuccess)
    {
        printError(retCode,__FILE__,__LINE__);
        return -1;
    }

    uint32_t nbrBuffersMaxMin = std::min(nbrBuffers, static_cast<uint32_t>(MAX_NUM_BUFFERS));
    nbrBuffersMaxMin = std::max(nbrBuffersMaxMin,static_cast<uint32_t>(MIN_NUM_BUFFERS));
    for (uint32_t i = 0; i < nbrBuffersMaxMin; ++i)
    {
        auto buff = std::make_shared<std::vector<int8_t> >(bufferLen);
        postBuffer(buff);
    }

    retCode = AlazarStartCapture(boardHandle);
    if( retCode != ApiSuccess)
    {
        printError(retCode,__FILE__,__LINE__);
    }
    rxThread = std::thread( &AlazarATS9870::rx, this );
    threadRunning = true;

    return 0;

}

void AlazarATS9870::rxThreadStop( void )
{
    if( threadRunning)
    {
        threadStop = true;
        rxThread.join();
        threadRunning = false;

        RETURN_CODE retCode = AlazarAbortAsyncRead(boardHandle);
        if( retCode != ApiSuccess)
        {
            printError(retCode,__FILE__,__LINE__);
        }
    }

    FILE_LOG(logDEBUG4) << "STOPPING RX THREAD" ;
    threadStop = false;
}

int32_t AlazarATS9870::postBuffer( shared_ptr<std::vector<int8_t>> buff)
{
    while (!bufferQ.push(buff));
    RETURN_CODE retCode = AlazarPostAsyncBuffer(boardHandle,buff.get()->data(),bufferLen);

    if( retCode != ApiSuccess)
    {
        printError(retCode,__FILE__,__LINE__);
        return(-1);
    }
    else
    {
        FILE_LOG(logDEBUG4) << "POSTED BUFFER " << std::hex << (uint64_t)(buff.get());
    }

    return(0);

}

int32_t AlazarATS9870::getBufferSize(void)
{

    //need to fit at least one record in a buffer
    if( recordLength * numChannels > bufferSize )
    {
        FILE_LOG(logERROR) << "SINGLE RECORD TO LARGE FOR THE BUFFER";
        return(-1);
    }

    //Find the factors of the number of round robins.
    //Each buffer will contain an integer number of round robins and the number
    //of round robins will be divided equally amoung all of the buffers.
    std::list<uint32_t> rrFactors;
    for(uint32_t ii = 1; ii<=nbrRoundRobins; ii++) {
        if(nbrRoundRobins % ii == 0) {
            rrFactors.push_front(ii);
        }
    }

    //find the maximum number of rr's that can fit in the buffer
    roundRobinsPerBuffer=0;
    for (std::list<uint32_t>::iterator rr=rrFactors.begin(); rr != rrFactors.end(); ++rr)
    {
        uint64_t bufferSizeTest = recordLength * nbrSegments * nbrWaveforms * *rr * numChannels;
        if ( bufferSizeTest <= bufferSize )
        {
            roundRobinsPerBuffer = *rr;
            FILE_LOG(logINFO) << "roundRobinsPerBuffer: " << roundRobinsPerBuffer;
            break;
        }
    }

    // if test is 0 then the round robin needs to be divided evenly into an
    // an integer number of buffers
    if( roundRobinsPerBuffer >= 1)
    {
        //compute the buffer size
        bufferLen = recordLength * nbrSegments * nbrWaveforms * roundRobinsPerBuffer * numChannels;
        FILE_LOG(logINFO) << "bufferLen: " << bufferLen;

        nbrBuffers = nbrRoundRobins/roundRobinsPerBuffer;
        FILE_LOG(logINFO) << "nbrBuffers: " << nbrBuffers;

        recordsPerBuffer = nbrSegments * nbrWaveforms * roundRobinsPerBuffer;
        FILE_LOG(logINFO) << "recordsPerBuffer: " << recordsPerBuffer;

        recordsPerAcquisition = recordsPerBuffer * nbrBuffers;
        FILE_LOG(logINFO) << "recordsPerAcquisition: " << recordsPerAcquisition;

        partialBuffer = false;
        FILE_LOG(logINFO) << "partialBuffer: " << partialBuffer;

        return(0);
    }

    // A round robin must be equally divided into buffers
    // Factor the number of records per round robin.
    // The number of records per buffer will be one of these factors
    uint32_t recordsPerRoundRobin = nbrSegments*nbrWaveforms;
    std::list<uint32_t> recFactors;
    for(uint32_t ii = 1; ii<=recordsPerRoundRobin; ii++) {
        if(recordsPerRoundRobin % ii == 0) {
            recFactors.push_front(ii);
        }
    }

    //Find the number of records that can come closest to the max buffer size
    buffersPerRoundRobin = 0;
    for (std::list<uint32_t>::iterator rec=recFactors.begin(); rec != recFactors.end(); ++rec)
    {
        uint32_t bufferSizeTest = recordLength * *rec * numChannels;
        if ( bufferSizeTest <= bufferSize )
        {
            recordsPerBuffer = *rec;
            FILE_LOG(logINFO) << "recordsPerBuffer: " << recordsPerBuffer;
            break;
        }
    }

    bufferLen = recordLength * recordsPerBuffer * numChannels;
    FILE_LOG(logINFO) << "bufferLen: " << bufferLen;

    recordsPerAcquisition = nbrSegments * nbrWaveforms * nbrRoundRobins;
    FILE_LOG(logINFO) << "recordsPerAcquisition: " << recordsPerAcquisition;

    nbrBuffers = recordsPerAcquisition/recordsPerBuffer;
    FILE_LOG(logINFO) << "nbrBuffers: " << nbrBuffers;

    partialBuffer = true;
    FILE_LOG(logINFO) << "partialBuffer: " << partialBuffer;

    buffersPerRoundRobin = nbrBuffers/nbrRoundRobins;
    FILE_LOG(logINFO) << "buffersPerRoundRobin: " << buffersPerRoundRobin;

    if( buffersPerRoundRobin*recordsPerBuffer*recordLength > MAX_WORK_BUFFER_SIZE )
    {
        FILE_LOG(logERROR) << " Exeeded MAX_WORK_BUFFER_SIZE";
        return(-1);
    }

    return(0);
}


int32_t AlazarATS9870::processBuffer( std::shared_ptr<std::vector<int8_t>> buffPtr, float *ch1, float *ch2)
{


    //accumulate the average in the application buffer which needs to
    //be cleared to start
    memset(ch1,0,sizeof(float)*samplesPerAcquisition);
    memset(ch2,0,sizeof(float)*samplesPerAcquisition);
    
    //the raw pointer makes the code more readable
    int8_t *buff = static_cast<int8_t*>(buffPtr.get()->data());

    if(averager)
    {
        // copy and sum along the 2nd and 4th dimension
        uint32_t ni = recordLength;
        uint32_t nj = nbrWaveforms;
        uint32_t nk = nbrSegments;
        uint32_t nl = roundRobinsPerBuffer;

    	for (uint32_t l=0; l < nl; l++)
        {
            for (uint32_t k=0; k < nk ; k++)
            {
                for (uint32_t j=0; j < nj; j++)
                {
                    for (uint32_t i=0; i < ni ; i++)
                    {
                        //ch1 and ch2 samples are interleaved for faster transfer times
                        ch1[i + k*ni] += counts2Volts*(buff[2*i     + j*2*ni + k*2*ni*nj + l*2*ni*nj*nk] - 128) - channelOffset;
                        ch2[i + k*ni] += counts2Volts*(buff[2*i + 1 + j*2*ni + k*2*ni*nj + l*2*ni*nj*nk] - 128) - channelOffset;
                    }
                }
            }
        }

        float denom=nj*nl;
        for( uint32_t i=0; i < ni*nk; i++)
        {
            ch1[i] /= denom;
            ch2[i] /= denom;
        }
    }
    else // digitizer mode
    {
        for( uint32_t i=0; i < bufferLen/2; i++)
        {
            ch1[i] = counts2Volts*(buff[2*i] - 128)   - channelOffset;
            ch2[i] = counts2Volts*(buff[2*i+1] - 128) - channelOffset;
        }
    }

    return 1;

}

int32_t AlazarATS9870::processPartialBuffer( std::shared_ptr<std::vector<int8_t>> buffPtr, float *ch1, float *ch2)
{
    uint32_t partialIndex = bufferCounter % buffersPerRoundRobin;
    FILE_LOG(logDEBUG4) << "PARTIAL INDEX " << partialIndex;
    bufferCounter++;

    //the raw pointer makes the code more readable
    int8_t *buff = static_cast<int8_t*>(buffPtr.get()->data());

    if( averager )
    {

        //process the buff into the work buffer and if it is the last buffer
        //in the round robin, run the averager
        float *pCh1 = (float *)(ch1WorkBuff.data() + bufferLen*partialIndex/2);
        float *pCh2 = (float *)(ch2WorkBuff.data() + bufferLen*partialIndex/2);

        for( uint32_t i=0; i < bufferLen/2; i++)
        {
            pCh1[i] = counts2Volts*(buff[2*i] - 128)   - channelOffset;
            pCh2[i] = counts2Volts*(buff[2*i+1] - 128) - channelOffset;
        }

        if( partialIndex == buffersPerRoundRobin - 1)
        {
            //accumulate the average in the application buffer which needs to
            //be cleared to start
            memset(ch1,0,sizeof(float)*samplesPerAcquisition);
            memset(ch2,0,sizeof(float)*samplesPerAcquisition);

            uint32_t ni = recordLength;
            uint32_t nj = nbrWaveforms;
            uint32_t nk = nbrSegments;

            //run the averager
            for (uint32_t k=0; k < nk ; k++)
            {
                for (uint32_t j=0; j < nj; j++)
                {
                    for (uint32_t i=0; i < ni ; i++)
                    {
                        //ch1 and ch2 samples are interleaved for faster transfer times
                        ch1[i + k*ni] += ch1WorkBuff[i + j*ni + k*ni*nj];
                        ch2[i + k*ni] += ch2WorkBuff[i + j*ni + k*ni*nj];
                    }
                }
            }

            float denom=nj;
            for( uint32_t i=0; i < ni*nk; i++)
            {
                ch1[i] /= denom;
                ch2[i] /= denom;
            }


        }
    }
    else
    {
        float *pCh1 = (float *)(ch1 + bufferLen*partialIndex/2);
        float *pCh2 = (float *)(ch2 + bufferLen*partialIndex/2);

        for( uint32_t i=0; i < bufferLen/2; i++)
        {
            pCh1[i] = counts2Volts*(buff[2*i] - 128)   - channelOffset;
            pCh2[i] = counts2Volts*(buff[2*i+1] - 128) - channelOffset;
        }

    }


    if( partialIndex == buffersPerRoundRobin - 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }

}



void AlazarATS9870::printError(RETURN_CODE code, std::string file, int32_t line )
{

    FILE_LOG(logERROR) << "File: " << file << " Line: "<< line << " ERROR: " << std::to_string(code) << " " << AlazarErrorToText( code);
}

int32_t AlazarATS9870::sysInfo()
{
    //log the sdk RevisionNumber
    uint8_t major,minor,rev;
    RETURN_CODE retCode = AlazarGetSDKVersion(&major,&minor,&rev);
    if( retCode != ApiSuccess)
    {
        printError(retCode,__FILE__,__LINE__);
    }
    else
    {
        FILE_LOG(logINFO) << "ATS SDK Rev " << to_string(major)<< "."<<to_string(minor)<<"."<<to_string(rev) ;
    }

    //see how many boards are installed
    //todo - need to think aobout error handling here - create a info method
    uint32_t systemCount = AlazarNumOfSystems();
	  FILE_LOG(logINFO) << "System count    = " << to_string(systemCount);

    if( systemCount > 0)
    {
        for (uint32_t systemId = 1; systemId <= systemCount; systemId++)
        {
            if (!DisplaySystemInfo(systemId))
				return 1;
        }

    }

    return 0;

}

//---------------------------------------------------------------------------
//
// Function    :  DisplaySystemInfo
//
// Description :  Display information about this board system
//
//---------------------------------------------------------------------------

int32_t AlazarATS9870::DisplaySystemInfo(uint32_t systemId)
{
	uint32_t boardCount = AlazarBoardsInSystemBySystemID(systemId);
	if (boardCount == 0)
	{
		FILE_LOG(logERROR) << "No boards found in system";
		return -1;
	}

	HANDLE handle = AlazarGetSystemHandle(systemId);
	if (handle == NULL)
	{
		FILE_LOG(logERROR) << "AlazarGetSystemHandle system failed";
		return -1;
	}

	uint32_t boardType = AlazarGetBoardKind(handle);
	if (boardType == ATS_NONE || boardType >= ATS_LAST)
	{
		FILE_LOG(logERROR) << "Unknown board type " << boardType;
		return -1;
	}

	uint8_t driverMajor, driverMinor, driverRev;
	RETURN_CODE retCode = AlazarGetDriverVersion(&driverMajor, &driverMinor, &driverRev);
	if (retCode != ApiSuccess)
	{
		FILE_LOG(logERROR) << "AlazarGetDriverVersion failed " << AlazarErrorToText(retCode);
		return -1;
	}

	FILE_LOG(logINFO) << "System ID       = " << systemId;
	FILE_LOG(logINFO) << "Board type      = " << BoardTypeToText(boardType);
	FILE_LOG(logINFO) << "Board count     = " << boardCount;
	FILE_LOG(logINFO) << "Driver version  = " << to_string(driverMajor)<<"."<<to_string(driverMinor)<<"."<<to_string(driverRev);

	// Display informataion about each board in this board system

	for (uint32_t boardId = 1; boardId <= boardCount; boardId++)
	{
		if (!DisplayBoardInfo(systemId, boardId))
			return -1;
	}

	return 0;
}



//---------------------------------------------------------------------------
//
// Function    :  DisplayBoardInfo
//
// Description :  Display information about a board
//
//---------------------------------------------------------------------------

int32_t AlazarATS9870::DisplayBoardInfo(uint32_t systemId, uint32_t boardId)
{
	RETURN_CODE retCode;

    HANDLE handle = AlazarGetBoardBySystemID(systemId, boardId);
    if (handle == NULL)
    {
        FILE_LOG(logERROR) << "Open systemId " << systemId << " boardId "<<boardId<<" failed";
		return -1;
    }

    uint32_t samplesPerChannel;
    BYTE bitsPerSample;
    retCode = AlazarGetChannelInfo(handle, &samplesPerChannel, &bitsPerSample);
    if (retCode != ApiSuccess)
    {
        FILE_LOG(logERROR) << "AlazarGetChannelInfo failed -- " << AlazarErrorToText(retCode);
        return -1;
    }

	uint32_t aspocType;
    retCode = AlazarQueryCapability(handle, ASOPC_TYPE, 0, &aspocType);
    if (retCode != ApiSuccess)
    {
        FILE_LOG(logERROR) << "AlazarQueryCapability failed -- " << AlazarErrorToText(retCode);
		return -1;
    }

	BYTE cpldMajor;
	BYTE cpldMinor;
	retCode = AlazarGetCPLDVersion(handle, &cpldMajor, &cpldMinor);
	if (retCode != ApiSuccess)
	{
		FILE_LOG(logERROR)<< "AlazarGetCPLDVersion failed -- "<< AlazarErrorToText(retCode);
		return -1;
	}

	uint32_t serialNumber;
	retCode = AlazarQueryCapability(handle, GET_SERIAL_NUMBER, 0, &serialNumber);
	if (retCode != ApiSuccess)
	{
        FILE_LOG(logERROR)<<"AlazarQueryCapability failed -- "<< AlazarErrorToText(retCode);
		return -1;
	}

	uint32_t latestCalDate;
	retCode = AlazarQueryCapability(handle, GET_LATEST_CAL_DATE, 0, &latestCalDate);
	if (retCode != ApiSuccess)
	{
        FILE_LOG(logERROR) << "Error: AlazarQueryCapability failed -- "<< AlazarErrorToText(retCode);
		return -1;
	}

	FILE_LOG(logINFO) << "System ID                 = " << systemId;
	FILE_LOG(logINFO) << "Board ID                  = " << boardId;
	FILE_LOG(logINFO) << "Serial number             = " << setw(6) << serialNumber;
	FILE_LOG(logINFO) << "Bits per sample           = " << to_string(bitsPerSample);
	FILE_LOG(logINFO) << "Max samples per channel   = " << samplesPerChannel;
	FILE_LOG(logINFO) << "CPLD version              = " << to_string(cpldMajor) <<"."<<to_string(cpldMinor);
	FILE_LOG(logINFO) << "FPGA version              = " << ((aspocType >> 16) & 0xff) << "."<<((aspocType >> 24) & 0xf);
	FILE_LOG(logINFO) << "ASoPC signature           = " << setw(8) << hex << aspocType;
	FILE_LOG(logINFO) << "Latest calibration date   = " << latestCalDate;

	if (IsPcieDevice(handle))
	{
		// Display PCI Express link information

		uint32_t linkSpeed;
		retCode = AlazarQueryCapability(handle, GET_PCIE_LINK_SPEED, 0, &linkSpeed);
		if (retCode != ApiSuccess)
		{
			FILE_LOG(logERROR) <<"AlazarQueryCapability failed -- " << AlazarErrorToText(retCode);
			return -1;
		}

		uint32_t linkWidth;
		retCode = AlazarQueryCapability(handle, GET_PCIE_LINK_WIDTH, 0, &linkWidth);
		if (retCode != ApiSuccess)
		{
		FILE_LOG(logERROR) << "Error: AlazarQueryCapability failed -- " << AlazarErrorToText(retCode);
			return -1;
		}

		FILE_LOG(logINFO) << "PCIe link speed           = "<< 2.5*linkSpeed << " Gbps";
		FILE_LOG(logINFO) << "PCIe link width           = "<< linkWidth << " lanes";
	}

	// Toggling the LED on the PCIe/PCIe mounting bracket of the board

	int cycleCount = 2;
	uint32_t cyclePeriod_ms = 200;

	if (!FlashLed(handle, cycleCount, cyclePeriod_ms))
		return -1;

	return 0;
}

//---------------------------------------------------------------------------
//
// Function    :  IsPcieDevice
//
// Description :  Return 0 if board has PCIe host bus interface
//
//---------------------------------------------------------------------------

bool AlazarATS9870::IsPcieDevice(HANDLE handle)
{
	uint32_t boardType = AlazarGetBoardKind(handle);
	if (boardType >= ATS9462)
		return true;
	else
		return false;
}

//---------------------------------------------------------------------------
//
// Function    :  FlashLed
//
// Description :  Flash LED on board's PCI/PCIe mounting bracket
//
//---------------------------------------------------------------------------

int32_t AlazarATS9870::FlashLed(HANDLE handle, int32_t cycleCount, uint32_t cyclePeriod_ms)
{
	for (int cycle = 0; cycle < cycleCount; cycle++)
	{
        FILE_LOG(logDEBUG3) << "Flashing LED...";

        const int phaseCount = 2;
		uint32_t sleepPeriod_ms = cyclePeriod_ms / phaseCount;

		for (int phase = 0; phase < phaseCount; phase++)
		{
			uint32_t state = (phase == 0) ? LED_ON : LED_OFF;
			RETURN_CODE retCode = AlazarSetLED(handle, state);
			if (retCode != ApiSuccess)
            printError(retCode,__FILE__,__LINE__);


			if (sleepPeriod_ms > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepPeriod_ms));

		}
	}

	return 0;
}


//---------------------------------------------------------------------------
//
// Function    :  BoardTypeToText
//
// Description :  Convert board type Id to text
//
//---------------------------------------------------------------------------

string AlazarATS9870::BoardTypeToText(int boardType)
{
	string pszName;

	switch (boardType)
	{
	case ATS850:
		pszName = "ATS850";
		break;
	case ATS310:
		pszName = "ATS310";
		break;
	case ATS330:
		pszName = "ATS330";
		break;
	case ATS855:
		pszName = "ATS855";
		break;
	case ATS315:
		pszName = "ATS315";
		break;
	case ATS335:
		pszName = "ATS335";
		break;
	case ATS460:
		pszName = "ATS460";
		break;
	case ATS860:
		pszName = "ATS860";
		break;
	case ATS660:
		pszName = "ATS660";
		break;
	case ATS9461:
		pszName = "ATS9461";
		break;
	case ATS9462:
		pszName = "ATS9462";
		break;
	case ATS9850:
		pszName = "ATS9850";
		break;
	case ATS9870:
		pszName = "ATS9870";
		break;
	case ATS9310:
		pszName = "ATS9310";
		break;
	case ATS9325:
		pszName = "ATS9325";
		break;
	case ATS9350:
		pszName = "ATS9350";
		break;
	case ATS9351:
		pszName = "ATS9351";
		break;
	case ATS9410:
		pszName = "ATS9410";
		break;
	case ATS9440:
		pszName = "ATS9440";
		break;
	case ATS_NONE:
	default:
		pszName = "?";
	}

	return pszName;
}
