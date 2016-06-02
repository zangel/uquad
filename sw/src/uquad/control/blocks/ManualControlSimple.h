#ifndef UQUAD_CONTROL_BLOCKS_MANUAL_CONTROL_SIMPLE_H
#define UQUAD_CONTROL_BLOCKS_MANUAL_CONTROL_SIMPLE_H

#include "../Config.h"
#include "ManualControl.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    class ManualControlSimple
        : public ManualControl
    {
    public:
        ManualControlSimple();
        ~ManualControlSimple();
        
        bool isValid() const;
        
        system::error_code setDirection(Direction dir);
        system::error_code setThrust(Thrust thrust);
    
    protected:
        system::error_code prepare(asio::yield_context yctx);
        system::error_code update(asio::yield_context yctx);
        
    protected:
    };

} //namespace blocks    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCKS_MANUAL_CONTROL_SIMPLE_H
