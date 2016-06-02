#include "QuadMotorsThrustSimple.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    QuadMotorsThrustSimple::QuadMotorsThrustSimple()
        : QuadMotorsThrust()
        , m_ThrustMatrix(Mat4x4f::Zero())
        , m_TR(Vec3f::Zero())
    {
        setFrameOrientation(M_PI_4);
    }
    
    QuadMotorsThrustSimple::~QuadMotorsThrustSimple()
    {
    }
    
    bool QuadMotorsThrustSimple::isValid() const
    {
        return true;
    }
    
    system::error_code QuadMotorsThrustSimple::setFrameOrientation(float fo)
    {
        if(isPrepared())
            return base::makeErrorCode(base::kEInvalidState);
        
        float const cfo = std::cos(fo);
        float const sfo = std::sin(fo);
        
        m_ThrustMatrix <<
            1.0f,   cfo,    sfo,    1.0f,
            1.0f,   sfo,   -cfo,   -1.0f,
            1.0f,  -cfo,   -sfo,    1.0f,
            1.0f,  -sfo,    cfo,   -1.0f;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code QuadMotorsThrustSimple::prepare(asio::yield_context yctx)
    {
        if(system::error_code qmte = QuadMotorsThrust::prepare(yctx))
            return qmte;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code QuadMotorsThrustSimple::update(asio::yield_context yctx)
    {
        #if 1
        Vec3f rotationRate = m_ISRotationRateSetpoint.m_Value.cwiseMax(-Vec3f::Ones()).cwiseMin(Vec3f::Ones());
        float thrust = std::max(0.0f, std::min(1.0f, m_ISThrustSetpoint.m_Value));
        #else
        #if 0
        Vec3f rotationRate(0.0f, 0.0f, -1.0f);
        #else
        Vec3f rotationRate = m_ISRotationRateSetpoint.m_Value.cwiseMax(-Vec3f::Ones()).cwiseMin(Vec3f::Ones());
        #endif
        float thrust = 0.5f;
        #endif
        
        Vec4f output = Vec4f::Zero();
        
        #if 0
        float rollPitchYawMix[4];
        float rollPitchYawMixMin = 0.f, rollPitchYawMixMax = 0.0f;
        
        for(std::size_t m = 0; m < 4; m++)
        {
            rollPitchYawMix[m] = Vec3f(m_ThrustMatrix.row(m).tail<3>()).cwiseProduct(rotationRate).sum();

            rollPitchYawMixMin = std::min(rollPitchYawMixMin, rollPitchYawMix[m]);
            rollPitchYawMixMax = std::max(rollPitchYawMixMax, rollPitchYawMix[m]);
        }
        
        float const rollPitchYawMixRange = rollPitchYawMixMax - rollPitchYawMixMin;
        float thrustMin = 0.0f;
        float thrustMax = 1.0f;
        
        
        if(rollPitchYawMixRange > 1.0f)
        {
            float const thrustRange = thrustMax - thrustMin;
            float mixReduction = thrustRange / rollPitchYawMixRange;
            for(std::size_t m = 0; m < 4; ++m)
            {
                rollPitchYawMix[m] *= mixReduction;
            }
            
            if(mixReduction > 0.5f)
            {
                thrustMin = thrustMax = thrustMin + (thrustRange / 2);
            }
        }
        else
        {
            thrustMin = thrustMin + (rollPitchYawMixRange / 2);
            thrustMax = thrustMax - (rollPitchYawMixRange / 2);
        }
        
        for(std::size_t m = 0; m < 4; m++)
        {
            output(m) = rollPitchYawMix[m] + std::min(std::max(thrustInput * m_ThrustMatrix(m, 0), thrustMin), thrustMax);
            output(m) = std::min(std::max(output(m), 0.0f), 1.0f);
        }
        #else
        float thrustIncreaseFactor = 1.5f;
        float thrustDecreaseFactor = 0.6f;
    
        float minOut = 0.0f;
        float maxOut = 0.0f;
        
        for(std::size_t m = 0; m < 4; ++m)
        {
            float out = Vec2f(m_ThrustMatrix.row(m).segment<2>(1)).cwiseProduct(rotationRate.head<2>()).sum() + thrust;
            minOut = std::min(minOut, out);
            maxOut = std::max(maxOut, out);
        }

        float boost = 0.0f;
        float rollPitchScale = 1.0f;
        
        if(minOut < 0.0f && maxOut < 1.0f && -minOut <= 1.0f - maxOut)
        {
            float maxThrustDiff = thrust * thrustIncreaseFactor - thrust;
            if(maxThrustDiff >= -minOut)
            {
                boost = -minOut;
            }
            else
            {
                boost = maxThrustDiff;
                rollPitchScale = (thrust + boost) / (thrust - minOut);
            }
        }
        else
        if(maxOut > 1.0f && minOut > 0.0f && minOut >= maxOut - 1.0f)
        {
            float maxThrustDiff = thrust - thrustDecreaseFactor * thrust;
            if(maxThrustDiff >= maxOut - 1.0f)
            {
                boost = -(maxOut - 1.0f);
            }
            else
            {
                boost = -maxThrustDiff;
                rollPitchScale = (1 - (thrust + boost)) / (maxOut - thrust);
            }
        }
        else
        if(minOut < 0.0f && maxOut < 1.0f && -minOut > 1.0f - maxOut)
        {
            float maxThrustDiff = thrust * thrustIncreaseFactor - thrust;
            boost = std::min(std::max(-minOut - (1.0f - maxOut) / 2.0f, 0.0f), maxThrustDiff);
            rollPitchScale = (thrust + boost) / (thrust - minOut);
        }
        else
        if(maxOut > 1.0f && minOut > 0.0f && minOut < maxOut - 1.0f)
        {
            float maxThrustDiff = thrust - thrustDecreaseFactor * thrust;
            boost = std::min(std::max(-(maxOut - 1.0f - minOut) / 2.0f, -maxThrustDiff), 0.0f);
            rollPitchScale = (1 - (thrust + boost)) / (maxOut - thrust);
        }
        else
        if(minOut < 0.0f && maxOut > 1.0f)
        {
            boost = std::min(std::max(-(maxOut - 1.0f + minOut) / 2.0f, thrustDecreaseFactor * thrust - thrust), thrustIncreaseFactor * thrust - thrust);
            rollPitchScale = (thrust + boost) / (thrust - minOut);
        }
        
        for(std::size_t m = 0; m < 4; ++m)
        {
            float out =
                Vec2f(m_ThrustMatrix.row(m).segment<2>(1)).cwiseProduct(rotationRate.head<2>()).sum() * rollPitchScale +
                rotationRate(2) * m_ThrustMatrix(m, 3) +
                thrust +
                boost;
            
            if(out < 0.0f)
            {
                if(std::abs(m_ThrustMatrix(m,3)) <= std::numeric_limits<float>::epsilon())
                {
                    rotationRate(2) = 0.0f;
                }
                else
                {
                    rotationRate(2) = -
                        (
                            Vec2f(m_ThrustMatrix.row(m).segment<2>(1)).cwiseProduct(rotationRate.head<2>()).sum() * rollPitchScale +
                            thrust +
                            boost
                        ) / m_ThrustMatrix(m,3);
                }

            }
            else
            if(out > 1.0f)
            {
                float thrustReduction = std::min(0.15f, out - 1.0f);
                thrust -= thrustReduction;

                if(std::abs(m_ThrustMatrix(m,3)) <= std::numeric_limits<float>::epsilon())
                {
                    rotationRate(2) = 0.0f;
                }
                else
                {
                    rotationRate(2) =
                        (1.0f -
                            (
                                Vec2f(m_ThrustMatrix.row(m).segment<2>(1)).cwiseProduct(rotationRate.head<2>()).sum() * rollPitchScale +
                                thrust +
                                boost
                            )
                        ) / m_ThrustMatrix(m,3);
                }
            }
        }

        for(std::size_t m = 0; m < 4; ++m)
        {
            output(m) =
                Vec2f(m_ThrustMatrix.row(m).segment<2>(1)).cwiseProduct(rotationRate.head<2>()).sum() * rollPitchScale +
                rotationRate(2) * m_ThrustMatrix(m,3) +
                thrust +
                boost;

            output(m) = std::min(std::max(output(m), 0.0f), 1.0f);
            //outputs[i] = constrain(_idle_speed + (outputs[i] * (1.0f - _idle_speed)), _idle_speed, 1.0f);
        }
        m_OSMotorsPower.m_Value = output;

        #endif
        
        return base::makeErrorCode(base::kENoError);
    }
    
} //namespace blocks
} //namespace control
} //namespace uquad
