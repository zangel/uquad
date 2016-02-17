#ifndef CVTRACKER_COMMON_MSG_TRACKER_INFO_H
#define CVTRACKER_COMMON_MSG_TRACKER_INFO_H

#include <uquad/comm/Message.h>

namespace cvtracker
{
namespace common
{
namespace msg
{
    class TrackerInfo
        : public uquad::comm::Message
    {
    public:
        TrackerInfo();
        
        ePriority priority() const;
        
        std::vector<uquad::Vec2i> trackingFeatures;
        std::vector< std::pair<uquad::Vec2i, uquad::Vec2i> > trackingTrails;
        
        uint32_t referenceGridHalfSize;
        std::vector<uquad::Vec2i> referenceGrid;
        
    public:
        friend class boost::serialization::access;
        
        template<class Archive>
        void serialize(Archive &ar, const unsigned int) BOOST_USED;
    };
    
} //namespace msg
} //namespace common
} //namespace cvtracker

UQUAD_BASE_SERIALIZATION_EXPORT(cvtracker::common::msg::TrackerInfo)

#endif //CVTRACKER_COMMON_MSG_TRACKER_INFO_H