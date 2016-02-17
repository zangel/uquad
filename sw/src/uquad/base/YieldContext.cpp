#include "YieldContext.h"
#include "Error.h"

namespace uquad
{
namespace base
{
    YieldContext::YieldContext(asio::io_service &ios, asio::yield_context const &yield)
        : m_Timer(ios)
        , m_Yield(yield)
        , m_DoneResult(makeErrorCode(kENoError))
    {
    }
    
    YieldContext::~YieldContext()
    {
    }
    
    
    system::error_code YieldContext::wait(asio::steady_timer::time_point const &t)
    {
        m_Timer.expires_at(t);
        system::error_code err;
        m_Timer.async_wait(m_Yield[err]);
        if(err)
        {
            if(err == asio::error::operation_aborted)
            {
                return m_DoneResult;
            }
            else
            {
                return err;
            }
        }
        return makeErrorCode(kETimeOut);
    }
    
    system::error_code YieldContext::waitBlock(asio::steady_timer::time_point const &t, function<system::error_code ()> const &block)
    {
        system::error_code err;
        m_Timer.cancel(err);
        m_Timer.expires_at(t, err);
        if(system::error_code be = block())
            return be;

        m_Timer.async_wait(m_Yield[err]);
        if(err)
        {
            if(err == asio::error::operation_aborted)
            {
                return m_DoneResult;
            }
            else
            {
                return err;
            }
        }
        return makeErrorCode(kETimeOut);
    }

    void YieldContext::done()
    {
        done(makeErrorCode(kENoError));
    }
    
    void YieldContext::done(system::error_code result)
    {
        system::error_code err;
        m_DoneResult = result;
        m_Timer.cancel(err);
    }
    
    void YieldContext::cancel()
    {
        done(makeErrorCode(kEOperationAborted));
    }

} //namespace base
} //namespace uquad
