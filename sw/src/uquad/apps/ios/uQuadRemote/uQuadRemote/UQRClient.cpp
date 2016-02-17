#include "UQRClient.h"
#include <uquad/base/Error.h>
#include <future>


UQRClient::UQRClient()
    : m_bStarted(false)
    , uquad::base::Application()
    , m_MainRunloop()
    , m_MainRunloopThread()
    , m_CameraClient(uquad::intrusive_ptr<uquad::base::Runloop>(&m_MainRunloop))
{
    intrusive_ptr_add_ref(&m_MainRunloop);
    intrusive_ptr_add_ref(&m_CameraClient);
    m_CameraClient.setRemoteAddress(uquad::asio::ip::address::from_string("192.168.0.1"));
    //m_CameraClient.setRemoteAddress(uquad::asio::ip::address::from_string("192.168.51.100"));
    m_CameraClient.setRemoteVideoSampleRate(30.0f);
    m_CameraClient.setRemoteVideoSampleSize(uquad::Vec2i(480, 320));
    
}

UQRClient::~UQRClient()
{
    
}

UQRClient* UQRClient::instance()
{
    return dynamic_cast<UQRClient*>(uquad::base::Application::instance());
}

uquad::intrusive_ptr<uquad::base::Runloop> UQRClient::mainRunloop()
{
    return uquad::intrusive_ptr<uquad::base::Runloop>(&m_MainRunloop);
}

uquad::intrusive_ptr<uquad::comm::CameraClient> UQRClient::cameraClient()
{
    return uquad::intrusive_ptr<uquad::comm::CameraClient>(&m_CameraClient);
}


void UQRClient::start()
{
    if(m_bStarted)
        return;
    
    m_bStarted = true;
    m_MainRunloopThread = std::move(
        uquad::thread(
            uquad::bind(&uquad::base::Runloop::run, &m_MainRunloop)
        )
    );
    
    {
        std::promise<uquad::system::error_code> cse;
        m_CameraClient.start([&cse](uquad::system::error_code const &e) { cse.set_value(e); });
        auto cse_future  = cse.get_future();
        cse_future.wait();
    }
}

bool UQRClient::isStarted() const
{
    return m_bStarted;
}

void UQRClient::stop()
{
    if(!m_bStarted)
        return;
    
    {
        std::promise<uquad::system::error_code> cse;
        m_CameraClient.stop([&cse](uquad::system::error_code const &e) { cse.set_value(e); });
        auto cse_future  = cse.get_future();
        cse_future.wait();
    }
    
    m_MainRunloop.stop();
    m_MainRunloopThread.join();
    m_MainRunloop.reset();
}