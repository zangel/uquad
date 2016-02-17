#ifndef UQUAD_CONTROL_EKF2_RING_BUFFER_H
#define UQUAD_CONTROL_EKF2_RING_BUFFER_H

#include "../Config.h"
#include "../../base/Time.h"

namespace uquad
{
namespace control
{
namespace ekf2
{
    template <typename T, int N>
    class RingBuffer
    {
    public:
        RingBuffer()
            : m_Head(N - 1)
            , m_Size(0)
        {
            
        }
        
        ~RingBuffer()
        {
        }
        
        inline void push(T const &v)
        {
            std::size_t const pos = (m_Head + 1) % N;
            
            m_Buffer[pos] = v;
            m_Head = pos;
            m_Size = std::min<std::size_t>(m_Size + 1, N);
        }
        
        inline bool empty() const
        {
            return m_Size == 0;
        }
        
        inline bool full() const
        {
            return m_Size == N;
        }
        
        inline std::size_t size() const
        {
            return m_Size;
        }
        
        inline T& oldest()
        {
            return m_Buffer[(m_Head + N + 1 - m_Size) % N];
        }
        
        inline T const& oldest() const
        {
            return m_Buffer[(m_Head + N + 1 - m_Size) % N];
        }
        
        inline T& newest()
        {
            return m_Buffer[m_Head];
        }
        
        inline T const& newest() const
        {
            return m_Buffer[m_Head];
        }
        
        inline bool popOlderThan(uint64_t t, T &v)
        {
            for(std::size_t i = 0; i < m_Size; ++i)
            {
                std::size_t const pos = (m_Head + N - i) % N;
                if(t >= m_Buffer[pos].time_us && t - m_Buffer[pos].time_us < 100000)
                {
                    v = m_Buffer[pos];
                    m_Size = i;
                    return true;
                }
            }
            return false;
        }
        
    private:
        std::size_t m_Head;
        std::size_t m_Size;
        T m_Buffer[N];
    };
    
    
} //namespace ekf2
} //namespace control
} //namespace uquad


#endif //UQUAD_CONTROL_EKF2_RING_BUFFER_H
