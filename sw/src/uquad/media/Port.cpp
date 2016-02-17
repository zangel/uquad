#include "Port.h"
#include "../utility/Constant.h"
#include "../base/Error.h"

namespace uquad
{
namespace media
{
    Port::Port()
    {
    }
    
    Port::~Port()
    {
    }
    
    InputPort::InputPort()
        : Port()
        , PortDelegate()
    {
    }
    
    InputPort::~InputPort()
    {
    }
    
    SampleType InputPort::getSampleType() const
    {
        if(m_IncomingPort)
            return m_IncomingPort->getSampleType();
        
        return SampleType();
    }
    
    Sample InputPort::getSample() const
    {
        if(m_IncomingPort)
            return m_IncomingPort->getSample();
        
        return Sample();
    }
    
    float InputPort::getSampleRate() const
    {
        if(m_IncomingPort)
            return m_IncomingPort->getSampleRate();
        
        return 0.0f;
    }
    
    system::error_code InputPort::setIncomingPort(intrusive_ptr<OutputPort> const &op)
    {
        if(m_IncomingPort == op)
            return base::makeErrorCode(base::kENoError);
        
        if(op && !acceptsPort(op))
            return base::makeErrorCode(base::kEInvalidArgument);
        
        if(m_IncomingPort)
        {
            m_IncomingPort->portDelegate() -= this;
        }
        
        m_IncomingPort = op;
        
        if(m_IncomingPort)
        {
            m_IncomingPort->portDelegate() += this;
        }
        return base::makeErrorCode(base::kENoError);
    }
    
    void InputPort::onPortConfigurationChanged()
    {
        notifyOnPortConfigurationChanged();
    }
    
    OutputPort::OutputPort()
        : Port()
    {
    }
    
    OutputPort::~OutputPort()
    {
    }
    
    system::error_code OutputPort::addOutgoingPort(intrusive_ptr<InputPort> const &ip)
    {
        if(!ip)
            return base::makeErrorCode(base::kEInvalidArgument);
        
        if(ip->getIncomingPort())
            return base::makeErrorCode(base::kENotApplicable);
        
        if(m_OutgoingPorts.count(ip))
            return base::makeErrorCode(base::kEAlreadyExists);
        
        if(ip->setIncomingPort(intrusive_ptr<OutputPort>(this)))
           return base::makeErrorCode(base::kENotApplicable);
        
        m_OutgoingPorts.insert(ip);
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code OutputPort::removeOutgoingPort(intrusive_ptr<InputPort> const &ip)
    {
        if(!ip)
            return base::makeErrorCode(base::kEInvalidArgument);
        
        if(!ip->getIncomingPort())
            return base::makeErrorCode(base::kENotApplicable);

        if(m_OutgoingPorts.count(ip) == 0)
            return base::makeErrorCode(base::kENotFound);
        
        if(ip->setIncomingPort(intrusive_ptr<OutputPort>()))
           return base::makeErrorCode(base::kENotApplicable);
        
        m_OutgoingPorts.erase(ip);
        return base::makeErrorCode(base::kENoError);
    }
    
    void OutputPort::clearOutgoingPorts()
    {
        while(!m_OutgoingPorts.empty())
            removeOutgoingPort(*m_OutgoingPorts.begin());
    }
    
    VideoPort::VideoPort()
        : Port()
    {
    }
    
    VideoPort::~VideoPort()
    {
    }
    
    VideoInputPort::VideoInputPort()
        : Port()
        , InputPort()
        , VideoPort()
    {
    }
    
    VideoInputPort::~VideoInputPort()
    {
    }
    
    Vec2i VideoInputPort::getVideoSampleSize() const
    {
        if(intrusive_ptr<VideoOutputPort> vp = getIncoming<VideoOutputPort>())
            return vp->getVideoSampleSize();
        
        return utility::Constant<Vec2i>::value();
    }
    
    VideoOutputPort::VideoOutputPort()
        : Port()
        , OutputPort()
        , VideoPort()
    {
    }
    
    VideoOutputPort::~VideoOutputPort()
    {
    }
    
    
} //namespace media
} //namespace uquad