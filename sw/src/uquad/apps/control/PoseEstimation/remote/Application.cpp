#include "Application.h"
#include <uquad/hal/DeviceManager.h>
#include <uquad/base/Error.h>

namespace ctrlpe
{
namespace remote
{

    Application::Application()
        : base::Application()
        , m_RemoteControlServer(mainRunloop())
    {
        m_RemoteControlServer.remoteControlDelegate() += this;
    }
    
    Application::~Application()
    {
        m_RemoteControlServer.remoteControlDelegate() -= this;
    }
    
    
    void Application::buildOptions(po::options_description &desc)
    {
        desc.add_options();
    }
    
    system::error_code Application::prepare()
    {
        m_UQuad = SI(hal::DeviceManager).findDevice<hal::UQuad>("UQuad");
        if(!m_UQuad)
        {
            return base::makeErrorCode(base::kENoSuchDevice);
        }
        
        if(m_UQuad->open())
        {
            m_UQuad.reset();
            return base::makeErrorCode(base::kENoSuchDevice);
        }
        
        if(!m_UQuad->areMotorsRunning())
        {
            m_UQuad->armMotors();
        }

        //m_CurrSensorData.reset(new common::msg::UQuadSensorsData());
        m_UQuad->uquadDelegate() += this;
        m_RemoteControlServer.start();
        return base::makeErrorCode(base::kENoError);
    }
    
    void Application::cleanup()
    {
        m_UQuad->close();
        m_UQuad->uquadDelegate() -= this;
        m_UQuad.reset();
        
        m_RemoteControlServer.stop();
    }
    
    void Application::onRemoteControlConnected()
    {
        m_CurrSensorData.reset(new common::msg::UQuadSensorsData());
    }
    
    void Application::onRemoteControlMessageReceived(intrusive_ptr<comm::Message> msg)
    {
        if(intrusive_ptr<common::msg::UQuadMotorsPower> motorsPower = dynamic_pointer_cast<common::msg::UQuadMotorsPower>(msg))
        {
            handleUQuadMotorsPowerReceived(motorsPower);
        }
    }
    
    void Application::handleUQuadMotorsPowerReceived(intrusive_ptr<common::msg::UQuadMotorsPower> mp)
    {
        m_UQuad->setMotorsPower(
            mp->motorsPower(3),
            mp->motorsPower(2),
            mp->motorsPower(1),
            mp->motorsPower(0)
        );
    }
    
    void Application::onUQuadSensorsReady()
    {
        if(m_RemoteControlServer.isConnected())
        {
            m_CurrSensorData->uquadSensorsData.push_back(hal::UQuadSensorsData());
            m_UQuad->getSensorsData(m_CurrSensorData->uquadSensorsData.back());

            if(m_CurrSensorData->uquadSensorsData.size() == NUM_SAMPLES)
            {
                m_RemoteControlServer.sendMessage(m_CurrSensorData);

                m_CurrSensorData.reset(new common::msg::UQuadSensorsData());
            }
        }
    }
    
} //namespace ctrlpe
} //namespace cvcc
