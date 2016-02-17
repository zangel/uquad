#ifndef UQUAD_BASE_MEMORY_BUFFER_H
#define UQUAD_BASE_MEMORY_BUFFER_H

#include "Config.h"
#include "Buffer.h"

namespace uquad
{
namespace base
{
	class Memory;

	class MemoryBuffer
    	: public Buffer
    {
    public:
    	MemoryBuffer(intrusive_ptr<Memory> const &m, std::size_t o, std::size_t s);
    	virtual ~MemoryBuffer();

        bool isValid() const;
        void* data();
        std::size_t offset() const;
    	std::size_t size() const;
        

    private:
    	intrusive_ptr<Memory> m_Memory;
    	std::size_t m_Offset;
    	std::size_t m_Size;
    };

} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_MEMORY_BUFFER_H
