#include "ASharedMemory.h"
#include "../../base/Error.h"
#include <cutils/ashmem.h>
#include <cutils/atomic.h>

namespace uquad
{
namespace hal
{
namespace android
{
    ASharedMemory::ASharedMemory(std::size_t size, uint32_t flags, char const* name)
        : m_Fd(-1)
        , m_Size(0)
        , m_Offset(0)
        , m_Data(MAP_FAILED)
        , m_Flags(flags)
        , m_bNeedUnmap(false)
    {
        const size_t pagesize = getpagesize();
        size = ((size + pagesize-1) & ~(pagesize-1));
        int fd = ::ashmem_create_region(name == NULL ? "MemoryHeapBase" : name, size);

        if(fd >= 0)
        {
            if(mapFd(fd, size))
            {
                if(flags & READ_ONLY)
                {
                    ::ashmem_set_prot_region(fd, PROT_READ);
                }
            }
        }
    }

    ASharedMemory::~ASharedMemory()
    {
        release();
    }

    bool ASharedMemory::isValid() const
    {
        return m_Fd > 0;
    }

    void* ASharedMemory::data()
    {
        return m_Data;
    }

    std::size_t ASharedMemory::size() const
    {
        return m_Size;
    }

    system::error_code ASharedMemory::release()
    {
        int fd = ::android_atomic_or(-1, &m_Fd);
        if(fd >= 0)
        {
            if(m_bNeedUnmap)
            {
                ::munmap(m_Data, m_Size);
            }
            m_Data = 0;
            m_Size = 0;
            close(fd);
        }
        return base::makeErrorCode(base::kENoError);
    }

    bool ASharedMemory::mapFd(int fd, std::size_t size, uint32_t offset)
    {
        if((m_Flags & DONT_MAP_LOCALLY) == 0)
        {
            void* base = (uint8_t*)::mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, offset);
            if(base == MAP_FAILED)
            {
                close(fd);
                return false;
            }
            m_Data = base;
            m_bNeedUnmap = true;
        }
        else
        {
            m_Data = 0;
            m_bNeedUnmap = false;
        }
        m_Fd = fd;
        m_Size = size;
        m_Offset = offset;
        return true;
    }

} //namespace android
} //namespace hal
} //namespace uquad
