#include "IOService.h"
#include "../Log.h"

namespace uquad
{
namespace hal
{
namespace android
{
    IOService::IOService()
        : m_IOS()
        , m_Work(m_IOS)
        , m_Worker(bind(&asio::io_service::run, &m_IOS))
    {
        UQUAD_HAL_LOG(Debug) << "IOService::IOService: service started!";
    }

    IOService::~IOService()
    {
        m_IOS.stop();
        m_Worker.join();

        UQUAD_HAL_LOG(Debug) << "IOService::~IOService: service stopped!";
    }

} //namespace android
} //namespace hal
} //namespace uquad
