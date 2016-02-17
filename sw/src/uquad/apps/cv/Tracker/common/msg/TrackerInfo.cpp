#include "TrackerInfo.h"
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <uquad/utility/EigenMatrixSerialization.h>

namespace cvtracker
{
namespace common
{
namespace msg
{
    TrackerInfo::TrackerInfo()
        : uquad::comm::Message()
    {
    }
    
    uquad::comm::Message::ePriority TrackerInfo::priority() const
    {
        return uquad::comm::Message::kPriorityHigh;
    }
    
    template<class Archive>
    void TrackerInfo::serialize(Archive &ar, const unsigned int)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Message);
        ar & trackingFeatures;
        ar & trackingTrails;
        ar & referenceGridHalfSize;
        ar & referenceGrid;
    }
    
} //namespace msg
} //namespace common
} //namespace cvtracker

UQUAD_BASE_SERIALIZATION_EXPORT_IMPLEMENT(cvtracker::common::msg::TrackerInfo)