#include "UQuadSensorsData.h"
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <uquad/utility/EigenMatrixSerialization.h>

namespace ctrlpe
{
namespace common
{
namespace msg
{
    UQuadSensorsData::UQuadSensorsData()
        : uquad::comm::Message()
    {
    }
    
    uquad::comm::Message::ePriority UQuadSensorsData::priority() const
    {
        return uquad::comm::Message::kPriorityHigh;
    }
    
    template<class Archive>
    void UQuadSensorsData::serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
        ar & uquadSensorsData;
    }
    
} //namespace msg
} //namespace common
} //namespace ctrlpe

UQUAD_BASE_SERIALIZATION_EXPORT_IMPLEMENT(ctrlpe::common::msg::UQuadSensorsData)