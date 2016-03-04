#ifndef LIBALAZAR_H_
#define LIBALAZAR_H_

#include <stdint.h>
#include <string>
#include <iostream>
#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>
#include <map>
#include <thread>
#include <array>


#include "libAlazarConfig.h"
#include "libAlazarAPI.h"
#include "AlazarApi.h"
#include "AlazarError.h"
#include "AlazarCmd.h"

#define MAX_NUM_BUFFERS 32
#define MIN_NUM_BUFFERS  2
#define MAX_BUFFER_SIZE 0x400000 // 4M
#define MAX_WORK_BUFFER_SIZE 0x400000 // 4M

class AlazarATS9870
{

    public:

        std::atomic<bool> threadStop;
        std::atomic<bool> threadRunning;

        //these queues are used to manage the DMA buffer pointers passed to the
        //ATS DLL
        boost::lockfree::spsc_queue<uint8_t*,  boost::lockfree::capacity<MAX_NUM_BUFFERS>> bufferQ;
        boost::lockfree::spsc_queue<uint8_t*, boost::lockfree::capacity<MAX_NUM_BUFFERS>> dataQ;

        std::atomic<int32_t> bufferCounter;

        static std::map<RETURN_CODE,std::string> errorMap;

        //this working buffer is used for the partial buffer logic when a round
        //robin is distributed over multiple buffers

        std::array<float, MAX_WORK_BUFFER_SIZE> ch1WorkBuff;
        std::array<float, MAX_WORK_BUFFER_SIZE> ch2WorkBuff;

        bool averager;

        uint32_t bufferLen;
        bool partialBuffer;
        uint32_t roundRobinsPerBuffer;
        uint32_t buffersPerRoundRobin;
        float counts2Volts;
        float channelOffset;

        uint32_t recordLength;
        uint32_t nbrSegments;
        uint32_t nbrWaveforms;
        uint32_t nbrRoundRobins;
        uint32_t nbrBuffers;

        uint32_t samplesPerAcquisition;


        AlazarATS9870();
        ~AlazarATS9870();
        int32_t sysInfo( void );
        int32_t rxThreadRun( void );
        void rxThreadStop( void );

        int32_t postBuffer( uint8_t *buff);
        void printError(RETURN_CODE code, std::string file, int32_t line );
        int32_t    ConfigureBoard(uint32_t systemId, uint32_t boardId,
            const ConfigData_t *config, AcquisitionParams_t *acqParams);

        int32_t processBuffer( uint8_t *buff, float *ch1, float *ch2);
        int32_t processPartialBuffer( uint8_t *buff, float *ch1, float *ch2);

    protected:

        ConfigData_t config;
        std::thread rxThread;
        HANDLE boardHandle;

        int32_t DisplaySystemInfo( uint32_t);
        int32_t DisplayBoardInfo(uint32_t systemId, uint32_t boardId);
        bool IsPcieDevice(HANDLE handle);
        int32_t FlashLed(HANDLE handle, int32_t cycleCount, uint32_t cyclePeriod_ms);

        std::string BoardTypeToText(int boardType);
        int32_t rx( void );
        int32_t getBufferSize(void);

        //map mV input scale to RangeId
        float channelScale;
        std::map<uint32_t,uint32_t> rangeIdMap =
        {
            {  40,  INPUT_RANGE_PM_40_MV},
            { 100,  INPUT_RANGE_PM_100_MV},
            { 200,  INPUT_RANGE_PM_200_MV},
            { 400,  INPUT_RANGE_PM_400_MV},
            {1000,  INPUT_RANGE_PM_1_V},
            {2000,  INPUT_RANGE_PM_2_V},
            {4000,  INPUT_RANGE_PM_4_V},
        };

        //map coupling input
        std::map<std::string,uint32_t> couplingMap =
        {
            {"AC",1},
            {"DC",2},
        };

        //map bandwidth string to control value
        std::map<std::string,uint32_t> bamdwithMap =
        {
            {"Full"  ,0},
            {"20MHz" ,1},
        };

        std::map<std::string,uint32_t> triggerSourceMap =
        {
            {"A",0},
            {"B",1},
            {"Ext",2},
        };

        std::map<std::string,uint32_t> triggerSlopeMap =
        {
            {"rising",1},
            {"falling",2},
        };


        uint32_t bufferSize;
        const uint32_t maxOnboardMemory  = 0x10000000; //256MB
        const uint32_t numChannels       = 2;

        uint32_t recordsPerBuffer;
        uint32_t recordsPerAcquisition;

        std::map<std::string,bool> modeMap =
        {
            {"digitizer",false},
            {"averager",true},
        };

};


#endif
