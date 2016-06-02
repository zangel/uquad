#ifndef UQUAD_CONTROL_BLOCKS_QUAD_MOTORS_THRUST_SIMPLE_H
#define UQUAD_CONTROL_BLOCKS_QUAD_MOTORS_THRUST_SIMPLE_H

#include "../Config.h"
#include "QuadMotorsThrust.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    class QuadMotorsThrustSimple
        : public QuadMotorsThrust
    {
    public:
        QuadMotorsThrustSimple();
        ~QuadMotorsThrustSimple();
        
        bool isValid() const;
        
        system::error_code setFrameOrientation(float fo);
    
    protected:
        system::error_code prepare(asio::yield_context yctx);
        system::error_code update(asio::yield_context yctx);
        
    protected:
        Mat4x4f m_ThrustMatrix;
        Vec3f m_TR;
    };

} //namespace blocks    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCKS_QUAD_MOTORS_THRUST_SIMPLE_H
