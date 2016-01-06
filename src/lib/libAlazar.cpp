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

void AlazarATS9870::printError(RETURN_CODE code, std::string file, int32_t line )
{

    FILE_LOG(logERROR) << "File: " << file << " Line: "<< line << " ERROR: " << std::to_string(code) << " " << AlazarErrorToText( code);
}

int32_t AlazarATS9870::sysInfo()
{
    FILE_LOG(logDEBUG4) << "Constructing ... ";
    threadStop = false;
    threadRunning = false;
    bufferLen = (int32_t)1e6;

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

	int cycleCount = 10;
	uint32_t cyclePeriod_ms = 2000;

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
