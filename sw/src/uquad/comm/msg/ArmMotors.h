#ifndef UQUAD_COMM_MSG_ARM_MOTORS_H
#define UQUAD_COMM_MSG_ARM_MOTORS_H

#include "Config.h"
#include "../Message.h"

namespace uquad
{
namespace comm
{
namespace msg
{
    class ArmMotors
        : public Message
    {
    public:
        ArmMotors();
        ArmMotors(bool a);
        
        ePriority priority() const;
        
        
        bool arm;
        
    public:
        friend class boost::serialization::access;
        
        template<class Archive>
        void serialize(Archive &ar, const unsigned int) BOOST_USED;
    };

} //namespace msg
} //namespace comm
} //namespace uquad

UQUAD_BASE_SERIALIZATION_EXPORT(uquad::comm::msg::ArmMotors)

#endif //UQUAD_COMM_MSG_ARM_MOTORS_H