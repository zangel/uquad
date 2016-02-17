#ifndef UQUAD_COMM_MESSAGE_H
#define UQUAD_COMM_MESSAGE_H

#include "Config.h"
#include "../base/RefCounted.h"

namespace uquad
{
namespace comm
{
    class Message
        : public base::RefCounted
    {
    public:
        typedef enum
        {
            kPriorityLow = 0,
            kPriorityNormal,
            kPriorityHigh,
            kPriorityCount
        } ePriority;
        
        typedef chrono::high_resolution_clock time_clock_t;
        
        Message();
        ~Message();
        
        virtual ePriority priority() const;
        
        time_clock_t::time_point time;
        
    public:
        friend class boost::serialization::access;
        
        template<class Archive>
        void serialize(Archive &ar, const unsigned int) BOOST_USED;
    };
    
} //namespace comm
} //namespace uquad

UQUAD_BASE_SERIALIZATION_EXPORT(uquad::comm::Message)

#endif //UQUAD_COMM_MESSAGE_H



