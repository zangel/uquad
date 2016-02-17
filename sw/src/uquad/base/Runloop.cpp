#include "Runloop.h"
#include "Error.h"

namespace uquad
{
namespace base
{
    Runloop::Runloop()
        : m_IOS()
        , m_IOSWork(m_IOS)
    {
    }
    
    Runloop::~Runloop()
    {
        stop();
    }
    
    void Runloop::stop()
    {
        m_IOS.stop();
    }
    
    bool Runloop::isStoped() const
    {
        return m_IOS.stopped();
    }
    
    void Runloop::reset()
    {
        m_IOS.reset();
    }
    
    system::error_code Runloop::run()
    {
        while(true)
        {
            std::size_t nExecuted = m_IOS.run();
            if(m_IOS.stopped())
                break;
        
            if(!nExecuted)
                this_thread::yield();
        }
        return makeErrorCode(kENoError);
    }
    
    system::error_code Runloop::runAtMostOne()
    {
        std::size_t nExecuted = m_IOS.poll_one();
        return makeErrorCode(nExecuted?kENoError:kENotApplicable);
    }
    
    system::error_code Runloop::runUntil(chrono::system_clock::time_point const &t)
    {
        while(chrono::system_clock::now() <= t && !m_IOS.stopped())
        {
            std::size_t nExecuted = m_IOS.poll_one();
            if(!m_IOS.stopped() && !nExecuted)
                this_thread::sleep_for(chrono::milliseconds(1));
        }
        
        return makeErrorCode(kENoError);
    }
    
    system::error_code Runloop::runUntil(function<bool ()> const &cond)
    {
        while(!m_IOS.stopped() && cond())
        {
            std::size_t nExecuted = m_IOS.poll_one();
            if(!m_IOS.stopped() && !nExecuted)
                this_thread::sleep_for(chrono::milliseconds(1));
        }
        
        return makeErrorCode(kENoError);
    }
    
} //namespace base
} //namespace uquad
