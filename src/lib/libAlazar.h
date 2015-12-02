#ifndef LIBALAZAR_H_
#define LIBALAZAR_H_

#include <stdint.h>
#include <string>
#include <iostream>

#include "libAlazarConfig.h"


class AlazarATS9870
{
    
    public:
                
        ConfigData_t config;
        
        AlazarATS9870();
        ~AlazarATS9870();
                
        int32_t rx( void );
        
        //run data receiver in a background thread
        std::thread rxThread;
        void rxThreadRun( void );
        bool threadStop;
        bool threadRunning;
        
        void rxThreadStop( void );
    
};


#endif