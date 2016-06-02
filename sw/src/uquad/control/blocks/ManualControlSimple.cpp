#include "ManualControlSimple.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    ManualControlSimple::ManualControlSimple()
        : ManualControl()
    {
    }
    
    ManualControlSimple::~ManualControlSimple()
    {
    }
    
    bool ManualControlSimple::isValid() const
    {
        return true;
    }
    
    system::error_code ManualControlSimple::setDirection(Direction dir)
    {
        m_OSDirection.m_Value = dir;
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code ManualControlSimple::setThrust(Thrust thrust)
    {
        m_OSThrust.m_Value = thrust;
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code ManualControlSimple::prepare(asio::yield_context yctx)
    {
        if(system::error_code mce = ManualControl::prepare(yctx))
            return mce;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code ManualControlSimple::update(asio::yield_context yctx)
    {
        return base::makeErrorCode(base::kENoError);
    }
    
} //namespace blocks
} //namespace control
} //namespace uquad
