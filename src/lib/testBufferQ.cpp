#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <array>

#include "alazarBuff.h"
#include "catch.hpp"

AlazarBufferQ<std::shared_ptr<std::vector<int8_t>>> q;

#define TEST_BUFFER_SIZE 256
#define TEST_Q_SIZE 32

void pusher( void )
{
    for( uint32_t i=0; i < TEST_Q_SIZE; i++)
    {
        auto p = std::make_shared<std::vector<int8_t> >(TEST_BUFFER_SIZE);

        std::memset(p.get()->data(),i,TEST_BUFFER_SIZE); 
        q.push(p);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }
}

void lockQ( void )
{
    std::lock_guard<std::mutex> lock(q.qmtx);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

TEST_CASE( "Alazar Buffer Q", "[bufferq]" ) 
{
    
    std::thread t1(pusher);
    t1.detach();
    
    std::shared_ptr<std::vector<int8_t>> temp;
    for( uint32_t i=0; i < TEST_Q_SIZE; i++)
    {
        //verify the lock by holding off the pusher thread
        lockQ();
        while( q.pop(temp) == false );
        REQUIRE( temp.get()->data()[0] == i);              
    }    
    REQUIRE( q.pop(temp) == false );  

}