#include "PoseEstimation.h"
#include "../BlockLibrary.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    PoseEstimation::Registry::Registry(std::string const &n)
        : base::Object::Registry(n, typeid(PoseEstimation))
    {
    }
    
    PoseEstimation::PoseEstimation()
        : Block()
        , m_ISDT("dT", 0.0f)
        , m_ISTime("time", 0.0f)
        , m_ISVelocityRate("velocityRate", Vec3f::Zero())
        , m_ISRotationRate("rotationRate", Vec3f::Zero())
        , m_ISMagneticField("magneticField", Vec3f::Zero())
        , m_ISAltitude("altitude", 0.0f)
        , m_OSAttitude("attitude", Attitude::Identity())
        , m_OSPosition("position", Vec3f::Zero())
        , m_OSVelocity("velocity", Vec3f::Zero())
        , m_OSEarthMagneticField("earthMagneticField", Vec3f::Zero())
    {
        intrusive_ptr_add_ref(&m_ISDT); addInputPort(&m_ISDT);
        intrusive_ptr_add_ref(&m_ISTime); addInputPort(&m_ISTime);
        intrusive_ptr_add_ref(&m_ISVelocityRate); addInputPort(&m_ISVelocityRate);
        intrusive_ptr_add_ref(&m_ISRotationRate); addInputPort(&m_ISRotationRate);
        intrusive_ptr_add_ref(&m_ISMagneticField); addInputPort(&m_ISMagneticField);
        intrusive_ptr_add_ref(&m_ISAltitude); addInputPort(&m_ISAltitude);
        intrusive_ptr_add_ref(&m_OSAttitude); addOutputPort(&m_OSAttitude);
        intrusive_ptr_add_ref(&m_OSPosition); addOutputPort(&m_OSPosition);
        intrusive_ptr_add_ref(&m_OSVelocity); addOutputPort(&m_OSVelocity);
        intrusive_ptr_add_ref(&m_OSEarthMagneticField); addOutputPort(&m_OSEarthMagneticField);
    }
    
    PoseEstimation::~PoseEstimation()
    {
    }
    
    intrusive_ptr<InputSignal> PoseEstimation::inputSignalDT() const
    {
        return &m_ISDT;
    }
    
    intrusive_ptr<InputSignal> PoseEstimation::inputSignalTime() const
    {
        return &m_ISTime;
    }
    
    intrusive_ptr<InputSignal> PoseEstimation::inputSignalVelocityRate() const
    {
        return &m_ISVelocityRate;
    }
    
    intrusive_ptr<InputSignal> PoseEstimation::inputSignalRotationRate() const
    {
        return &m_ISRotationRate;
    }
    
    intrusive_ptr<InputSignal> PoseEstimation::inputSignalMagneticField() const
    {
        return &m_ISMagneticField;
    }
    
    intrusive_ptr<InputSignal> PoseEstimation::inputSignalAltitude() const
    {
        return &m_ISAltitude;
    }
    
    intrusive_ptr<OutputSignal> PoseEstimation::outputSignalAttitude() const
    {
        return &m_OSAttitude;
    }
    
    intrusive_ptr<OutputSignal> PoseEstimation::outputSignalPosition() const
    {
        return &m_OSPosition;
    }
    
    intrusive_ptr<OutputSignal> PoseEstimation::outputSignalVelocity() const
    {
        return &m_OSVelocity;
    }
    
    intrusive_ptr<OutputSignal> PoseEstimation::outputSignalEarthMagneticField() const
    {
        return &m_OSEarthMagneticField;
    }
    
    system::error_code PoseEstimation::prepare(asio::yield_context yctx)
    {
        if(system::error_code bpe = Block::prepare(yctx))
            return bpe;
        
        m_ISDT.m_Value = 0.0f;
        m_ISTime.m_Value = 0.0f;
        m_ISVelocityRate.m_Value.setZero();
        m_ISRotationRate.m_Value.setZero();
        m_ISMagneticField.m_Value.setZero();
        m_ISAltitude.m_Value = 0.0f;
        m_OSAttitude.m_Value.setIdentity();
        m_OSPosition.m_Value.setZero();
        m_OSVelocity.m_Value.setZero();
        m_OSEarthMagneticField.m_Value.setZero();
        
        return base::makeErrorCode(base::kENoError);
    }
    
} //namespace blocks
} //namespace control
} //namespace uquad

#if defined(UQUAD_CONTROL_BLOCKS_PE_EKF)
#include "pe/ekf/PoseEstimation.h"
UQUAD_BASE_REGISTER_OBJECT(uquad::control::blocks::pe::ekf::PoseEstimation, uquad::control::BlockLibrary)
#endif

#if defined(UQUAD_CONTROL_BLOCKS_PE_EKF2)
#include "pe/ekf2/PoseEstimation.h"
UQUAD_BASE_REGISTER_OBJECT(uquad::control::blocks::pe::ekf2::PoseEstimation, uquad::control::BlockLibrary)
#endif

#if defined(UQUAD_CONTROL_BLOCKS_PE_IKF)
#include "pe/ikf/PoseEstimation.h"
UQUAD_BASE_REGISTER_OBJECT(uquad::control::blocks::pe::ikf::PoseEstimation, uquad::control::BlockLibrary)
#endif

#if defined(UQUAD_CONTROL_BLOCKS_PE_MADGWICK)
#include "pe/madgwick/PoseEstimation.h"
UQUAD_BASE_REGISTER_OBJECT(uquad::control::blocks::pe::madgwick::PoseEstimation, uquad::control::BlockLibrary)
#endif
