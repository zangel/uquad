#ifndef UQUAD_BASE_YIELD_EVENT_H
#define UQUAD_BASE_YIELD_EVENT_H

#include "Config.h"
#include "RefCounted.h"


namespace uquad
{
namespace base
{
    class YieldEvent
        : public RefCounted
    {
    public:
        YieldEvent(asio::io_service &ios);
        ~YieldEvent();
        
        system::error_code wait(asio::yield_context &ctx, asio::steady_timer::time_point const &t);
        inline system::error_code wait(asio::yield_context &ctx)
        {
            return wait(ctx, asio::steady_timer::time_point::max());
        }
        
        template <class Rep, class Period>
        inline system::error_code waitFor(asio::yield_context &ctx, std::chrono::duration<Rep, Period> const &p)
        {
            return wait(ctx, asio::steady_timer::clock_type::now() + p);
        }
        
        system::error_code waitBlock(asio::yield_context &ctx, asio::steady_timer::time_point const &t, function<system::error_code ()> const &block);

        inline system::error_code waitBlock(asio::yield_context &ctx, function<system::error_code ()> const &block)
        {
            return waitBlock(ctx, asio::steady_timer::time_point::max(), block);
        }

        template <class Rep, class Period>
        inline system::error_code waitBlockFor(asio::yield_context &ctx, std::chrono::duration<Rep, Period> const &p, function<system::error_code ()> const &block)
        {
            return waitBlock(ctx, asio::steady_timer::clock_type::now() + p, block);
        }

        void set(system::error_code result);
        void set();

        void reset();
        
    private:
        asio::io_service &m_IOS;
        asio::steady_timer m_Timer;
        system::error_code m_Result;
    };
    
    
} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_YIELD_EVENT_H
