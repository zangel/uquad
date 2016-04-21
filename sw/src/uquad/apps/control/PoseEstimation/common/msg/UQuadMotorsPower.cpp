#include "UQuadMotorsPower.h"
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <uquad/utility/EigenMatrixSerialization.h>

namespace ctrlpe
{
namespace common
{
namespace msg
{
    UQuadMotorsPower::UQuadMotorsPower()
        : uquad::comm::Message()
    {
    }
    
    uquad::comm::Message::ePriority UQuadMotorsPower::priority() const
    {
        return uquad::comm::Message::kPriorityHigh;
    }
    
    template<class Archive>
    void UQuadMotorsPower::serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
        ar & motorsPower;
    }
    
} //namespace msg
} //namespace common
} //namespace ctrlpe

UQUAD_BASE_SERIALIZATION_EXPORT_IMPLEMENT(ctrlpe::common::msg::UQuadMotorsPower)