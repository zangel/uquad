#ifndef UQUAD_HAL_ANDROID_ASHARED_MEMORY_H
#define UQUAD_HAL_ANDROID_ASHARED_MEMORY_H

#include "../Config.h"
#include "../../base/Memory.h"


namespace uquad
{
namespace hal
{
namespace android
{

    class ASharedMemory
        : public base::Memory
    {
    public:
        enum
        {
            READ_ONLY = 0x00000001,
            DONT_MAP_LOCALLY = 0x00000100,
            NO_CACHING = 0x00000200
        };

        ASharedMemory(std::size_t size, uint32_t flags = 0, char const* name = 0);
        ~ASharedMemory();

        bool isValid() const;
        void* data();
        std::size_t size() const;

        system::error_code release();

    private:
        bool mapFd(int fd, std::size_t size, uint32_t offset = 0);

    private:
        int m_Fd;
        std::size_t m_Size;
        uint32_t    m_Offset;
        void *m_Data;
        uint32_t m_Flags;
        bool m_bNeedUnmap;
    };

} //namespace android
} //namespace hal
} //namespace uquad


#endif //UQUAD_HAL_ANDROID_ASHARED_MEMORY_H
