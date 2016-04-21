#include "Gyroscope.h"

namespace uquad
{
namespace control
{
    std::string const& Gyroscope::rotationRateName()
    {
        static std::string _name = "rotationRate";
        return _name;
    }
    
    Gyroscope::Gyroscope()
        : Block()
        , m_RotationRate(rotationRateName(), RotationRate::Zero())
//        , m_CalibrationTransform(Transform3af::Identity())
    {
        intrusive_ptr_add_ref(&m_RotationRate); addOutputPort(&m_RotationRate);
    }
    
    Gyroscope::~Gyroscope()
    {
    }
    
/*    bool Gyroscope::isValid() const
    {
        return true;
    }
    
    void Gyroscope::setRotationRate(RotationRate const &rr)
    {
        m_RotationRate.m_Value = m_CalibrationTransform * rr;
    }
    
    void Gyroscope::setCalibrationTransform(Transform3af const &ct)
    {
        m_CalibrationTransform = ct;
    }
    
    system::error_code Gyroscope::prepare(asio::yield_context yctx)
    {
        if(system::error_code bpe = Block::prepare(yctx))
            return bpe;
        
        setRotationRate(RotationRate::Zero());
        
        return base::makeErrorCode(base::kENoError);
    }
    */
    
} //namespace control
} //namespace uquad
