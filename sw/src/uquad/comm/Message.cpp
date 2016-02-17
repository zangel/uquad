#include "Message.h"

namespace uquad
{
namespace comm
{
    Message::Message()
        : time(time_clock_t::now())
    {
    }
    
    Message::~Message()
    {
    }
    
    template<class Archive>
    void Message::serialize(Archive &ar, const unsigned int)
    {
        ar & time;
    }
    
    Message::ePriority Message::priority() const
    {
        return kPriorityNormal;
    }
    
    
} //namespace comm
} //namespace uquad

UQUAD_BASE_SERIALIZATION_EXPORT_IMPLEMENT(uquad::comm::Message)
