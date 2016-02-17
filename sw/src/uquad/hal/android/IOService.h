#ifndef UQUAD_HAL_ANDROID_IO_SERVICE_H
#define UQUAD_HAL_ANDROID_IO_SERVICE_H

#include "../Config.h"

namespace uquad
{
namespace hal
{
namespace android
{
    class IOService
    {
    public:
    protected:
        IOService();

    public:
        inline bool isStarted() const { return !m_IOS.stopped(); }

        inline asio::io_service& ios() { return m_IOS; }

    public:
       ~IOService();

    private:
       asio::io_service m_IOS;
       asio::io_service::work m_Work;
       thread m_Worker;
    };


} //namespace android
} //namespace hal
} //namespace uquad

#endif //UQUAD_HAL_ANDROID_GRALLOC_H
