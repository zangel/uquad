#ifndef CTRLPE_COMMON_MSG_UQUAD_SENSORS_DATA_H
#define CTRLPE_COMMON_MSG_UQUAD_SENSORS_DATA_H

#include <uquad/comm/Message.h>
#include <uquad/hal/UQuad.h>

namespace ctrlpe
{
namespace common
{
namespace msg
{
    class UQuadSensorsData
        : public uquad::comm::Message
    {
    public:
        UQuadSensorsData();
        
        ePriority priority() const;
        
        
        std::vector<uquad::hal::UQuadSensorsData> uquadSensorsData;
        
    public:
        friend class boost::serialization::access;
        
        template<class Archive>
        void serialize(Archive &ar, const unsigned int) BOOST_USED;
    };
    
} //namespace msg
} //namespace common
} //namespace ctrlpe

UQUAD_BASE_SERIALIZATION_EXPORT(ctrlpe::common::msg::UQuadSensorsData)

#endif //CTRLPE_COMMON_MSG_UQUAD_SENSORS_DATA_H