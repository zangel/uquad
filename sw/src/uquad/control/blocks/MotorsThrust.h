#ifndef UQUAD_CONTROL_BLOCKS_MOTORS_THRUST_H
#define UQUAD_CONTROL_BLOCKS_MOTORS_THRUST_H

#include "../Config.h"
#include "../Block.h"
#include "../Signal.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    class MotorsThrust
        : public virtual Block
    {
    public:
        MotorsThrust();
        ~MotorsThrust();
        
    protected:
    };

} //namespace blocks
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCKS_MOTORS_THRUST_H
