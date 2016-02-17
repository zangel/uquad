#ifndef UQUAD_BASE_MEMORY_H
#define UQUAD_BASE_MEMORY_H

#include "Config.h"
#include "RefCounted.h"

namespace uquad
{
namespace base
{
    class Memory
    	: public RefCounted
    {
    public:
    	Memory();
    	virtual ~Memory();

        virtual bool isValid() const = 0;
        virtual void* data() = 0;
    	virtual std::size_t size() const = 0;
    	virtual system::error_code release() = 0;
    };

} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_MEMORY_H
