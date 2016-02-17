#include "YieldEvent.h"
#include "Error.h"

namespace uquad
{
namespace base
{
    YieldEvent::YieldEvent(asio::io_service &ios)
        : m_IOS(ios)
        , m_Timer(ios)
        , m_Result(makeErrorCode(kENoError))
    {
    }
    
    YieldEvent::~YieldEvent()
    {
    }
    
    system::error_code YieldEvent::wait(asio::yield_context &ctx, asio::steady_timer::time_point const &t)
    {
        m_Result = makeErrorCode(kETimeOut);
        system::error_code err;
        m_Timer.expires_at(t, err);

        if(err)
            return makeErrorCode(kEInvalidState);
        
        m_Timer.async_wait(ctx[err]);

        if(err)
        {
            if(err == asio::error::operation_aborted)
            {
                return m_Result;
            }
            else
            {
                return err;
            }
        }
        return makeErrorCode(kETimeOut);
    }
    
    system::error_code YieldEvent::waitBlock(asio::yield_context &ctx, asio::steady_timer::time_point const &t, function<system::error_code ()> const &block)
    {
        m_Result = makeErrorCode(kETimeOut);
        system::error_code err;
        m_Timer.expires_at(t, err);
        if(err)
            return makeErrorCode(kEInvalidState);
        
        if((err = block()))
            return err;

        m_Timer.async_wait(ctx[err]);

        if(err)
        {
            if(err == asio::error::operation_aborted)
            {
                return m_Result;
            }
            else
            {
                return err;
            }
        }
        return makeErrorCode(kETimeOut);
    }
    
    void YieldEvent::set(system::error_code result)
    {
        m_IOS.post([this, result]()
        {
            m_Result = result;
            system::error_code err;
            m_Timer.cancel(err);
        });
    }

    void YieldEvent::set()
    {
        set(makeErrorCode(kENoError));
    }
    
    void YieldEvent::reset()
    {
        set(makeErrorCode(kEOperationAborted));
    }

} //namespace base
} //namespace uquad
