#ifndef CTRLPE_COMMON_MSG_UQUAD_MOTORS_POWER_H
#define CTRLPE_COMMON_MSG_UQUAD_MOTORS_POWER_H

#include <uquad/comm/Message.h>
#include <uquad/hal/UQuad.h>

namespace ctrlpe
{
namespace common
{
namespace msg
{
    class UQuadMotorsPower
        : public uquad::comm::Message
    {
    public:
        UQuadMotorsPower();
        
        ePriority priority() const;
        
        
        uquad::Vec4f motorsPower;
        
    public:
        friend class boost::serialization::access;
        
        template<class Archive>
        void serialize(Archive &ar, const unsigned int) BOOST_USED;
    };
    
} //namespace msg
} //namespace common
} //namespace ctrlpe

UQUAD_BASE_SERIALIZATION_EXPORT(ctrlpe::common::msg::UQuadMotorsPower)

#endif //CTRLPE_COMMON_MSG_UQUAD_MOTORS_POWER_H