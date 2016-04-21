#include "Clock.h"

namespace uquad
{
namespace control
{
    std::string const& Clock::timeName()
    {
        static std::string _name = "time";
        return _name;
    }
    
    std::string const& Clock::dTName()
    {
        static std::string _name = "dT";
        return _name;
    }
    
    Clock::Clock()
        : Block()
        , m_Time(timeName(), 0.0f)
        , m_DT(dTName(), 0.0f)
        //, m_FirstTimeStamp(base::TimePoint::min())
        //, m_LastTimeStamp(base::TimePoint::min())
    {
        intrusive_ptr_add_ref(&m_Time); addOutputPort(&m_Time);
        intrusive_ptr_add_ref(&m_DT); addOutputPort(&m_DT);
    }
    
    Clock::~Clock()
    {
    }
    
    /*bool Clock::isValid() const
    {
        return true;
    }
    
    void Clock::setTimeStamp(base::TimePoint ts)
    {
        if(m_FirstTimeStamp == base::TimePoint::min())
        {
            m_FirstTimeStamp = ts;
            m_LastTimeStamp = ts;
        }
        
        base::TimeDuration const absT = ts - m_FirstTimeStamp;
        base::TimeDuration const dT = ts - m_LastTimeStamp;
        
        m_LastTimeStamp = ts;
        
        m_Time.m_Value = chrono::duration_cast<chrono::microseconds>(absT).count() * 1.0e-6f;
        m_DT.m_Value = chrono::duration_cast<chrono::microseconds>(dT).count() * 1.0e-6f;
    }
    
    system::error_code Clock::prepare(asio::yield_context yctx)
    {
        if(system::error_code bpe = Block::prepare(yctx))
            return bpe;
        
        m_FirstTimeStamp = m_LastTimeStamp = base::TimePoint::min();
        m_Time.m_Value = 0.0f;
        m_DT.m_Value = 0.0f;
        
        return base::makeErrorCode(base::kENoError);
    }
    */

} //namespace control
} //namespace uquad
