#ifndef UQUAD_BASE_BUFFER_H
#define UQUAD_BASE_BUFFER_H

#include "Config.h"
#include "RefCounted.h"

namespace uquad
{
namespace base
{
	class Buffer
    	: public RefCounted
    {
    public:
    	Buffer();
    	virtual ~Buffer();
        
        virtual bool isValid() const = 0;
        virtual void* data() = 0;
        virtual std::size_t size() const = 0;
    };

} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_BUFFER_H
