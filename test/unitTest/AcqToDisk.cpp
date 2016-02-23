//---------------------------------------------------------------------------
//
// Copyright (c) 2008-2010 AlazarTech, Inc.
//
// AlazarTech, Inc. licenses this software under specific terms and
// conditions. Use of any of the software or derivatives thereof in any
// product without an AlazarTech digitizer board is strictly prohibited.
//
// AlazarTech, Inc. provides this software AS IS, WITHOUT ANY WARRANTY,
// EXPRESS OR IMPLIED, INCLUDING, WITHOUT LIMITATION, ANY WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. AlazarTech makes no
// guarantee or representations regarding the use of, or the results of the
// use of, the software and documentation in terms of correctness, accuracy,
// reliability, currentness, or otherwise; and you rely on the software,
// documentation and results solely at your own risk.
//
// IN NO EVENT SHALL ALAZARTECH BE LIABLE FOR ANY LOSS OF USE, LOSS OF
// BUSINESS, LOSS OF PROFITS, INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL
// DAMAGES OF ANY KIND. IN NO EVENT SHALL ALAZARTECH'S TOTAL LIABILITY EXCEED
// THE SUM PAID TO ALAZARTECH FOR THE PRODUCT LICENSED HEREUNDER.
//
//---------------------------------------------------------------------------

// AcqToDisk.cpp :
//
// This program demonstrates how to configure an ATS9870 to make a
// No-PreTrigger (NPT) AutoDMA acquisition.
//

#include <stdio.h>
#include <conio.h>
#include "AlazarError.h"
#include "AlazarApi.h"
#include "AlazarCmd.h"

// TODO: Select the number of DMA buffers to allocate.

#define BUFFER_COUNT 10

#define TRUE 1
#define FALSE 0

// Globals variables

U8 *BufferArray[BUFFER_COUNT] = { NULL };

// Forward declarations

BOOL ConfigureBoard(HANDLE boardHandle);
BOOL AcquireData(HANDLE boardHandle);

//----------------------------------------------------------------------------
//
// Function    :  main
//
// Description :  Program entry point
//
//----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	// TODO: Select a board

	U32 systemId = 1;
	U32 boardId = 1;

	// Get a handle to the board

	HANDLE boardHandle = AlazarGetBoardBySystemID(systemId, boardId);
	if (boardHandle == NULL)
	{
		printf("Error: Unable to open board system Id %u board Id %u\n", systemId, boardId);
		return 1;
	}

	// Configure the board's sample rate, input, and trigger settings

	if (!ConfigureBoard(boardHandle))
	{
		printf("Error: Configure board failed\n");
		return 1;
	}

	// Make an acquisition, optionally saving sample data to a file

	if (!AcquireData(boardHandle))
	{
		printf("Error: Acquisition failed\n");
		return 1;
	}

	return 0;
}

//----------------------------------------------------------------------------
//
// Function    :  ConfigureBoard
//
// Description :  Configure sample rate, input, and trigger settings
//
//----------------------------------------------------------------------------

BOOL ConfigureBoard(HANDLE boardHandle)
{
	RETURN_CODE retCode;

	// TODO: Specify the sample rate (see sample rate id below)

	double samplesPerSec = 500.e6;

	// TODO: Select clock parameters as required to generate this sample rate.
	//
	// For example: if samplesPerSec is 100.e6 (100 MS/s), then:
	// - select clock source INTERNAL_CLOCK and sample rate SAMPLE_RATE_100MSPS
	// - select clock source FAST_EXTERNAL_CLOCK, sample rate SAMPLE_RATE_USER_DEF,
	//   and connect a 100 MHz signalto the EXT CLK BNC connector.

	retCode =
		AlazarSetCaptureClock(
			boardHandle,			// HANDLE -- board handle
			//INTERNAL_CLOCK,			// U32 -- clock source id
            EXTERNAL_CLOCK_10MHz_REF,
			1000000000,		// U32 -- sample rate id
			CLOCK_EDGE_RISING,		// U32 -- clock edge id
			2						// U32 -- clock decimation
			);
	if (retCode != ApiSuccess)
	{
		printf("Error: AlazarSetCaptureClock failed -- %s\n", AlazarErrorToText(retCode));
		return FALSE;
	}

	// TODO: Select CHA input parameters as required
	retCode =
		AlazarInputControl(
			boardHandle,			// HANDLE -- board handle
			CHANNEL_A,				// U8 -- input channel
			AC_COUPLING,			// U32 -- input coupling id
			INPUT_RANGE_PM_4_V,		// U32 -- input range id
			IMPEDANCE_50_OHM		// U32 -- input impedance id
			);
	if (retCode != ApiSuccess)
	{
		printf("Error: AlazarInputControl failed -- %s\n", AlazarErrorToText(retCode));
		return FALSE;
	}

	// TODO: Select CHA bandwidth limit as required

	retCode =
		AlazarSetBWLimit(
			boardHandle,			// HANDLE -- board handle
			CHANNEL_A,				// U8 -- channel identifier
			0						// U32 -- 0 = disable, 1 = enable
			);
	if (retCode != ApiSuccess)
	{
		printf("Error: AlazarSetBWLimit failed -- %s\n", AlazarErrorToText(retCode));
		return FALSE;
	}

	// TODO: Select CHB input parameters as required

	retCode =
		AlazarInputControl(
			boardHandle,			// HANDLE -- board handle
			CHANNEL_B,				// U8 -- channel identifier
			AC_COUPLING,			// U32 -- input coupling id
			INPUT_RANGE_PM_4_V,		// U32 -- input range id
			IMPEDANCE_50_OHM		// U32 -- input impedance id
			);
	if (retCode != ApiSuccess)
	{
		printf("Error: AlazarInputControl failed -- %s\n", AlazarErrorToText(retCode));
		return FALSE;
	}

	// TODO: Select CHB bandwidth limit as required

	retCode =
		AlazarSetBWLimit(
			boardHandle,			// HANDLE -- board handle
			CHANNEL_B,				// U8 -- channel identifier
			0						// U32 -- 0 = disable, 1 = enable
			);
	if (retCode != ApiSuccess)
	{
		printf("Error: AlazarSetBWLimit failed -- %s\n", AlazarErrorToText(retCode));
		return FALSE;
	}

    // TODO: Select external trigger parameters as required

	retCode =
		AlazarSetExternalTrigger(
			boardHandle,			// HANDLE -- board handle
			DC_COUPLING,			// U32 -- external trigger coupling id
			ETR_5V					// U32 -- external trigger range id
			);
	// TODO: Select trigger inputs and levels as required

	retCode =
		AlazarSetTriggerOperation(
			boardHandle,			// HANDLE -- board handle
			TRIG_ENGINE_OP_J,		// U32 -- trigger operation
			TRIG_ENGINE_J,			// U32 -- trigger engine id
			TRIG_EXTERNAL,			// U32 -- trigger source id
			TRIGGER_SLOPE_POSITIVE,	// U32 -- trigger slope id
			154,					// U32 -- trigger level from 0 (-range) to 255 (+range)
			TRIG_ENGINE_K,			// U32 -- trigger engine id
			TRIG_DISABLE,			// U32 -- trigger source id for engine K
			TRIGGER_SLOPE_POSITIVE,	// U32 -- trigger slope id
			128						// U32 -- trigger level from 0 (-range) to 255 (+range)
			);
	if (retCode != ApiSuccess)
	{
		printf("Error: AlazarSetTriggerOperation failed -- %s\n", AlazarErrorToText(retCode));
		return FALSE;
	}


	// TODO: Set trigger delay as required.

	double triggerDelay_sec = 0.;
	U32 triggerDelay_samples = (U32) (triggerDelay_sec * samplesPerSec + 0.5);
	retCode = AlazarSetTriggerDelay(boardHandle, triggerDelay_samples);
	if (retCode != ApiSuccess)
	{
		printf("Error: AlazarSetTriggerDelay failed -- %s\n", AlazarErrorToText(retCode));
		return FALSE;
	}

	// TODO: Set trigger timeout as required.

	// NOTE:
	// The board will wait for a for this amount of time for a trigger event.
	// If a trigger event does not arrive, then the board will automatically
	// trigger. Set the trigger timeout value to 0 to force the board to wait
	// forever for a trigger event.
	//
	// IMPORTANT:
	// The trigger timeout value should be set to zero after appropriate
	// trigger parameters have been determined, otherwise the
	// board may trigger if the timeout interval expires before a
	// hardware trigger event arrives.

	double triggerTimeout_sec = 0.;
	U32 triggerTimeout_clocks = (U32) (triggerTimeout_sec / 10.e-6 + 0.5);

	retCode =
		AlazarSetTriggerTimeOut(
			boardHandle,			// HANDLE -- board handle
			0	// U32 -- timeout_sec / 10.e-6 (0 means wait forever)
			);
	if (retCode != ApiSuccess)
	{
		printf("Error: AlazarSetTriggerTimeOut failed -- %s\n", AlazarErrorToText(retCode));
		return FALSE;
	}

	// TODO: Configure AUX I/O connector as required
#if 0
	retCode =
		AlazarConfigureAuxIO(
			boardHandle,			// HANDLE -- board handle
			AUX_OUT_TRIGGER,		// U32 -- mode
			0						// U32 -- parameter
			);
	if (retCode != ApiSuccess)
	{
		printf("Error: AlazarConfigureAuxIO failed -- %s\n", AlazarErrorToText(retCode));
		return FALSE;
	}
#endif
	return TRUE;
}

//----------------------------------------------------------------------------
//
// Function    :  AcquireData
//
// Description :  Perform an acquisition, optionally saving data to file.
//
//----------------------------------------------------------------------------

BOOL AcquireData(HANDLE boardHandle)
{
	// There are no pre-trigger samples in NPT mode

	U32 preTriggerSamples = 0;

	// TODO: Select the number of post-trigger samples per record

	U32 postTriggerSamples = 4096;

	// TODO: Specify the number of records per DMA buffer

	U32 recordsPerBuffer = 1;

	// TODO: Specify the acquisition time in seconds.

	double acquistionTime_sec = 100.;

	// TODO: Select which channels to capture (A, B, or both)

	U32 channelMask = CHANNEL_A | CHANNEL_B;

	// TODO: Select if you wish to save the sample data to a file

	BOOL saveData = FALSE;

	// Calculate the number of enabled channels from the channel mask

	int channelCount = 0;

	switch (channelMask)
	{
	case CHANNEL_A:
	case CHANNEL_B:
		channelCount = 1;
		break;
	case CHANNEL_A | CHANNEL_B:
		channelCount = 2;
		break;
	default:
		printf("Error: Invalid channel mask %08X\n", channelMask);
		return FALSE;
	}

	// Get the sample size in bits, and the on-board memory size in samples per channel

	U8 bitsPerSample;
	U32 maxSamplesPerChannel;
	RETURN_CODE retCode = AlazarGetChannelInfo(boardHandle, &maxSamplesPerChannel, &bitsPerSample);
	if (retCode != ApiSuccess)
	{
		printf("Error: AlazarGetChannelInfo failed -- %s\n", AlazarErrorToText(retCode));
		return FALSE;
	}

	// Calculate the size of each DMA buffer in bytes

	U32 bytesPerSample = (bitsPerSample + 7) / 8;
	U32 samplesPerRecord = preTriggerSamples + postTriggerSamples;
	U32 bytesPerRecord = bytesPerSample * samplesPerRecord;
	U32 bytesPerBuffer = bytesPerRecord * recordsPerBuffer * channelCount;

	// Create a data file if required

	FILE *fpData = NULL;

	if (saveData)
	{
		fpData = fopen("data.bin", "wb");
		if (fpData == NULL)
		{
			printf("Error: Unable to create data file -- %u\n", GetLastError());
			return FALSE;
		}
	}

	// Allocate memory for DMA buffers

	BOOL success = TRUE;

	U32 bufferIndex;
	for (bufferIndex = 0; (bufferIndex < BUFFER_COUNT) && success; bufferIndex++)
	{
#ifdef _WIN32	// Allocate page aligned memory
		BufferArray[bufferIndex] = (U8*) VirtualAlloc(NULL, bytesPerBuffer, MEM_COMMIT, PAGE_READWRITE);
#else
		BufferArray[bufferIndex] = (U8*) malloc(bytesPerBuffer);
#endif
		if (BufferArray[bufferIndex] == NULL)
		{
			printf("Error: Alloc %u bytes failed\n", bytesPerBuffer);
			success = FALSE;
		}
	}

	// Configure the record size

	if (success)
	{
		retCode =
			AlazarSetRecordSize (
				boardHandle,			// HANDLE -- board handle
				preTriggerSamples,		// U32 -- pre-trigger samples
				postTriggerSamples		// U32 -- post-trigger samples
				);
		if (retCode != ApiSuccess)
		{
			printf("Error: AlazarSetRecordSize failed -- %s\n", AlazarErrorToText(retCode));
			success = FALSE;
		}
	}

	// Configure the board to make an NPT AutoDMA acquisition

	if (success)
	{
		//U32 recordsPerAcquisition = 0x7fffffff;			// Acquire until aborted
        U32 recordsPerAcquisition = 1;			// Acquire until aborted

		U32 admaFlags = ADMA_EXTERNAL_STARTCAPTURE |	// Start acquisition when AlazarStartCapture is called
						ADMA_NPT;						// Acquire multiple records with no-pretrigger samples

		retCode =
			AlazarBeforeAsyncRead(
				boardHandle,			// HANDLE -- board handle
				channelMask,			// U32 -- enabled channel mask
				-(long)preTriggerSamples,	// long -- offset from trigger in samples
				samplesPerRecord,		// U32 -- samples per record
				recordsPerBuffer,		// U32 -- records per buffer
				recordsPerAcquisition,	// U32 -- records per acquisition
				admaFlags				// U32 -- AutoDMA flags
				);
		if (retCode != ApiSuccess)
		{
			printf("Error: AlazarBeforeAsyncRead failed -- %s\n", AlazarErrorToText(retCode));
			success = FALSE;
		}
        printf("BUFFERS PER RECORD: %d\n",recordsPerBuffer);
        printf("SAMPLES PER RECORD: %d\n",samplesPerRecord);
        fflush(stdout);
	}

	// Add the buffers to a list of buffers available to be filled by the board
    printf("BYTES PER BUFFER: %d\n",bytesPerBuffer);
    fflush(stdout);
	for (bufferIndex = 0; (bufferIndex < BUFFER_COUNT) && success; bufferIndex++)
	{
		U8* pBuffer = BufferArray[bufferIndex];
		retCode = AlazarPostAsyncBuffer(boardHandle, pBuffer, bytesPerBuffer);
		if (retCode != ApiSuccess)
		{
			printf("Error: AlazarPostAsyncBuffer %u failed -- %s\n", bufferIndex, AlazarErrorToText(retCode));
			success = FALSE;
		}
	}

	// Arm the board system to wait for a trigger event to begin the acquisition

	if (success)
	{
		retCode = AlazarStartCapture(boardHandle);
		if (retCode != ApiSuccess)
		{
			printf("Error: AlazarStartCapture failed -- %s\n", AlazarErrorToText(retCode));
			success = FALSE;
		}
	}

	// Wait for each buffer to be filled, process the buffer, and re-post it to the board.

	if (success)
	{
		printf("Capturing for %g seconds ... press any key to abort\n", acquistionTime_sec);

		DWORD startTickCount = GetTickCount();
		DWORD stopTickCount = startTickCount + (DWORD) (acquistionTime_sec * 1000. + 0.5);
		U32 buffersCompleted = 0;
		INT64 bytesTransferred = 0;

		for (;;)
		{
			// TODO: Set a buffer timeout that is longer than the time
			//       required to capture all the records in one buffer.

			DWORD timeout_ms = 30000;

			// Poll until a buffer is full.
			// Note that the DMA buffers are filled in the order that they were posted.

			int bufferIndex = buffersCompleted % BUFFER_COUNT;
			U8* pBuffer = BufferArray[bufferIndex];

			DWORD timeoutTickCount = GetTickCount() + timeout_ms;

			BOOL bufferFull = FALSE;

			while (success && !bufferFull)
			{
				// Test if the buffer is full

				retCode = AlazarWaitAsyncBufferComplete(boardHandle, pBuffer, 1000);
				if (retCode == ApiSuccess)
				{
					// The buffer is full, exit polling loop
					bufferFull = TRUE;
                    printf("GOT ONE\n");
                    fflush(stdout);
				}
				else if (retCode != ApiWaitTimeout)
				{
					// The acquisition failed, exit polling loop
					printf("Error: AlazarWaitAsyncBufferComplete failed -- %s\n", AlazarErrorToText(retCode));
                    fflush(stdout);
					success = FALSE;
				}
				else if (GetTickCount() > timeoutTickCount)
				{
					// The polling timeout interval expired before the buffer was filled
					printf("Error: AlazarWaitAsyncBufferComplete timeout after polling for %lu ms\n", timeout_ms);
                    fflush(stdout);
					success = FALSE;
				}
				else
				{
					// The buffer is not full yet
					// TODO: Add code that should run during each iteration of the polling loop.
					//sSleep (1000);
                    printf("HELLO\n");
                    fflush(stdout);
				}
			}

			if (bufferFull)
			{
				// The buffer is full and has been removed from the list
				// of buffers available for the board

				buffersCompleted++;
				bytesTransferred += bytesPerBuffer;

				// TODO: Process sample data in this buffer.

				// NOTE:
				//
				// While you are processing this buffer, the board is already
				// filling the next available buffer(s).
				//
				// You MUST finish processing this buffer and post it back to the
				// board before the board fills all of its available DMA buffers
				// and on-board memory.
				//
				// Records are arranged in the buffer as follows:
				// R0A, R1A, R2A ... RnA, R0B, R1B, R2B ...
				//
				// Samples values are arranged contiguously in each record.
				// An 8-bit sample code is stored in each 8-bit sample value.
				//
				// Sample codes are unsigned by default. As a result:
				// - a sample code of 0x00 represents a negative full scale input signal.
				// - a sample code of 0x80 represents a ~0V signal.
				// - a sample code of 0xFF represents a positive full scale input signal.

				U8 *pRecord = pBuffer;

				for (int channel = 0; (channel < channelCount) && (success == TRUE); channel++)
				{
					for (U32 record = 0; (record < recordsPerBuffer) && (success == TRUE); record++)
					{
						if (saveData)
						{
							// Write record to file

							size_t bytesWritten = fwrite(pRecord, sizeof(BYTE), bytesPerRecord, fpData);
							if (bytesWritten != bytesPerRecord)
							{
								printf("Error: Write record %u failed -- %u\n", record, GetLastError());
								success = FALSE;
							}
						}

						// Seek to next record in buffer

						pRecord += samplesPerRecord;
					}
				}
			}

			// Add the buffer to the end of the list of available buffers.

			if (success)
			{
				retCode = AlazarPostAsyncBuffer(boardHandle, pBuffer, bytesPerBuffer);
				if (retCode != ApiSuccess)
				{
					printf("Error: AlazarPostAsyncBuffer failed -- %s\n", AlazarErrorToText(retCode));
					success = FALSE;
				}
			}

			// If the acquisition failed, exit the acquisition loop

			if (!success)
				break;

			// If a key was pressed, exit the acquisition loop

			if (_kbhit())
			{
				printf("Aborted...\n");
				break;
			}

			// If the acquistion period is over, abort the acquisition

			//if (GetTickCount() > stopTickCount)
            if( buffersCompleted >= 10)
            {
				printf("Acquistion complete.\n");
				break;
			}

			// Display progress

			//printf("Completed %u buffers\r", buffersCompleted);
		}

		// Display results

		double transferTime_sec = (GetTickCount() - startTickCount) / 1000.;
		printf("Capture completed in %.2lf sec\n", transferTime_sec);

		U32 recordsTransferred = recordsPerBuffer * buffersCompleted;

		double buffersPerSec;
		double bytesPerSec;
		double recordsPerSec;

		if (transferTime_sec > 0.)
		{
			buffersPerSec = buffersCompleted / transferTime_sec;
			bytesPerSec = bytesTransferred / transferTime_sec;
			recordsPerSec = recordsTransferred / transferTime_sec;

		}
		else
		{
			buffersPerSec = 0.;
			bytesPerSec = 0.;
			recordsPerSec = 0.;
		}

		printf("Captured %u buffers (%.4g buffers per sec)\n", buffersCompleted, buffersPerSec);
		printf("Captured %u records (%.4g records per sec)\n", recordsTransferred, recordsPerSec);
		printf("Transferred %I64d bytes (%.4g bytes per sec)\n", bytesTransferred, bytesPerSec);
	}

	// Abort the acquisition

	retCode = AlazarAbortAsyncRead(boardHandle);
	if (retCode != ApiSuccess)
	{
		printf("Error: AlazarAbortAsyncRead failed -- %s\n", AlazarErrorToText(retCode));
		success = FALSE;
	}

	// Free all memory allocated

	for (bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++)
	{
		if (BufferArray[bufferIndex] != NULL)
		{
#ifdef _WIN32
			VirtualFree(BufferArray[bufferIndex], 0, MEM_RELEASE);
#else
			free(BufferArray[bufferIndex]);
#endif
		}
	}

	// Close the data file

	if (fpData != NULL)
		fclose(fpData);

	return success;
}
