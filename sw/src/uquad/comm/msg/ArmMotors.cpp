#include "ArmMotors.h"


namespace uquad
{
namespace comm
{
namespace msg
{
    ArmMotors::ArmMotors()
        : Message()
        , arm(true)
    {
    }
    
    ArmMotors::ArmMotors(bool a)
        : Message()
        , arm(a)
    {
    }
    
    Message::ePriority ArmMotors::priority() const
    {
        return Message::kPriorityHigh;
    }
    
    template<class Archive>
    void ArmMotors::serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
        ar & arm;
    }
    
    
} //namespace msg
} //namespace comm
} //namespace uquad

UQUAD_BASE_SERIALIZATION_EXPORT_IMPLEMENT(uquad::comm::msg::ArmMotors)
