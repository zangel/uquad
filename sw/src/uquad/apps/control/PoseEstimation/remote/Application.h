#ifndef CTRLPE_REMOTE_APPLICATION_H
#define CTRLPE_REMOTE_APPLICATION_H

#include <uquad/Config.h>
#include <uquad/base/Application.h>
#include <uquad/base/Runloop.h>
#include <uquad/hal/UQuad.h>
#include <uquad/comm/RemoteControlServer.h>
#include "../common/msg/UQuadSensorsData.h"
#include "../common/msg/UQuadMotorsPower.h"


namespace ctrlpe
{
namespace remote
{
    using namespace uquad;
    
    class Application
        : public base::Application
        , protected comm::RemoteControlDelegate
        , protected hal::UQuadDelegate
    {
    public:
        Application();
        ~Application();
        
        void buildOptions(po::options_description &desc);
        system::error_code prepare();
        void cleanup();
        
        static uint32_t const NUM_SAMPLES = 10;
        
    protected:
        void onRemoteControlConnected();
        void onRemoteControlMessageReceived(intrusive_ptr<comm::Message> msg);
        void handleUQuadMotorsPowerReceived(intrusive_ptr<common::msg::UQuadMotorsPower> mp);
        void onUQuadSensorsReady();
        
        
    private:
        intrusive_ptr<hal::UQuad> m_UQuad;
        comm::RemoteControlServer m_RemoteControlServer;
        intrusive_ptr<common::msg::UQuadSensorsData> m_CurrSensorData;
    };
    
} //namespace remote
} //namespace ctrlpe


#endif //CTRLPE_REMOTE_APPLICATION_H