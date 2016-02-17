#ifndef UQUAD_BASE_RUNLOOP_H
#define UQUAD_BASE_RUNLOOP_H

#include "Config.h"
#include "RefCounted.h"

namespace uquad
{
namespace base
{
    class Runloop
        : public RefCounted
    {
    public:
        Runloop();
        ~Runloop();
        
        asio::io_service& ios() { return m_IOS; }
        
        void stop();
        bool isStoped() const;
        void reset();
        
        system::error_code run();
        system::error_code runAtMostOne();
        system::error_code runUntil(chrono::system_clock::time_point const &t);
        system::error_code runUntil(function<bool ()> const &cond);
        
        template <class Rep, class Period>
        inline system::error_code runFor(chrono::duration<Rep,Period> const &d)
        {
            return runUntil(chrono::system_clock::now() + d);
        }
        
        
        
    private:
        asio::io_service m_IOS;
        asio::io_service::work m_IOSWork;
    };
    
} //namespace base
} //namespace uquad

#endif //UQUAD_BASE_RUNLOOP_H