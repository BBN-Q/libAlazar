#ifndef ALAZARBUFF_H_
#define ALAZARBUFF_H_

#include <mutex>
#include <queue>

template <typename T>
class AlazarBufferQ
{
    
    public:
        
        std::queue<T> q;
        std::mutex qmtx;
    
        bool push( T &ptr )
        {
            std::lock_guard<std::mutex> lock(qmtx);
            q.push(ptr);    
            return true;
        }
        
        bool pop( T &ptr )
        {
            std::lock_guard<std::mutex> lock(qmtx);
            
            if( q.empty() )
            {
                return false;
            }
            
            ptr = q.front();
            q.pop();
            
            return true;
        }
        
        void clear(T &ptr)
        {
            while(!q.empty())
            {
                ptr = q.front();
                q.pop();                
            }
        }

};

#endif