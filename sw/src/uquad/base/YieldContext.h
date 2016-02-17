#ifndef UQUAD_BASE_YIELD_CONTEXT_H
#define UQUAD_BASE_YIELD_CONTEXT_H

#include "Config.h"
#include "RefCounted.h"


namespace uquad
{
namespace base
{
    class YieldContext
        : public RefCounted
    {
    public:
        YieldContext(asio::io_service &ios, asio::yield_context const &yield);
        ~YieldContext();
        
        system::error_code wait(asio::steady_timer::time_point const &t);
        
        inline system::error_code wait() { return wait(asio::steady_timer::time_point::max());}
        
        template <class Rep, class Period>
        inline system::error_code waitFor(chrono::duration<Rep, Period> const &p)
        {
            return wait(asio::steady_timer::clock_type::now() + p);
        }
        
        system::error_code waitBlock(asio::steady_timer::time_point const &t, function<system::error_code ()> const &block);

        inline system::error_code waitBlock(function<system::error_code ()> const &block) { return waitBlock(asio::steady_timer::time_point::max(), block); }

        template <class Rep, class Period>
        inline system::error_code waitBlockFor(chrono::duration<Rep, Period> const &p, function<system::error_code ()> const &block)
        {
            return waitBlock(asio::steady_timer::clock_type::now() + p, block);
        }

        void done();
        void done(system::error_code result);
        
        void cancel();
        
    private:
        asio::steady_timer m_Timer;
        asio::yield_context m_Yield;
        system::error_code m_DoneResult;
    };
    
    
} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_YIELD_CONTEXT_H
