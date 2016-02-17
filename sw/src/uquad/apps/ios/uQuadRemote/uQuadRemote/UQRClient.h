#ifndef UQR_CLIENT_H
#define UQR_CLIENT_H

#include <uquad/base/Application.h>
#include <uquad/base/Runloop.h>
#include <uquad/comm/CameraClient.h>


class UQRClient
    : public uquad::base::Application
{
public:
    UQRClient();
    ~UQRClient();
    
    static UQRClient* instance();
    
    uquad::intrusive_ptr<uquad::base::Runloop> mainRunloop();
    
    uquad::intrusive_ptr<uquad::comm::CameraClient> cameraClient();
    
    void start();
    bool isStarted() const;
    void stop();
    
    
private:
    uquad::atomic<bool> m_bStarted;
    uquad::base::Runloop m_MainRunloop;
    uquad::thread m_MainRunloopThread;
    uquad::comm::CameraClient m_CameraClient;
};

#endif //UQR_CLIENT_H