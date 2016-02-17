#ifndef UQUAD_BASE_REF_COUNTED_H
#define UQUAD_BASE_REF_COUNTED_H

#include "Config.h"

namespace uquad
{
namespace base
{
	class RefCounted
    {
    public:
        RefCounted();
        virtual ~RefCounted();
        
    public:
        mutable thread_safe_counter::type m_RefCounter;
    };
    
    static inline void intrusive_ptr_add_ref(const RefCounted *rc)
    {
        thread_safe_counter::increment(rc->m_RefCounter);
    }
    
    static inline void intrusive_ptr_release(const RefCounted *rc)
    {
        if(thread_safe_counter::decrement(rc->m_RefCounter) == 0)
            delete static_cast<const RefCounted*>(rc);
    }
    
    

} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_REF_COUNTED_H
