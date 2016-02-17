#include "MemoryBuffer.h"
#include "Memory.h"

namespace uquad
{
namespace base
{
    MemoryBuffer::MemoryBuffer(intrusive_ptr<Memory> const &m, std::size_t o, std::size_t s)
        : m_Memory()
        , m_Offset(0)
        , m_Size(0)
    {
        if(m && m->isValid() && o < m->size() && (o + s) <= m->size())
        {
            m_Memory = m;
            m_Offset = o;
            m_Size = s;
        }
    }
    
    MemoryBuffer::~MemoryBuffer()
    {
        m_Memory.reset();
    }
    
    bool MemoryBuffer::isValid() const
    {
        return m_Memory.get() && m_Memory->isValid();
    }
    
    void* MemoryBuffer::data()
    {
        if(!isValid())
            return 0;
        
        return reinterpret_cast<uint8_t*>(m_Memory->data()) + m_Offset;
    }
    
    std::size_t MemoryBuffer::offset() const
    {
        return m_Offset;
    }
    
    std::size_t MemoryBuffer::size() const
    {
        return m_Size;
    }
    
    
} //namespace base
} //namespace uquad
