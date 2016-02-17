#ifndef UQUAD_MEDIA_PORT_H
#define UQUAD_MEDIA_PORT_H

#include "Config.h"
#include "../base/RefCounted.h"
#include "../base/Delegate.h"
#include "SampleType.h"
#include "Sample.h"
#include "../base/Buffer.h"


namespace uquad
{
namespace media
{
    class InputPort;
    class OutputPort;
    class Sink;
    class Source;
    
    class PortDelegate
	{
	public:
		PortDelegate() = default;
		virtual ~PortDelegate() = default;
        
        virtual void onPortConfigurationChanged() {}
	};
    
    class Port
    	: public base::RefCounted
    {
    public:
        typedef base::Delegate<PortDelegate> port_delegate_t;
        
    	Port();
    	~Port();
        
        virtual SampleType getSampleType() const = 0;
        virtual Sample getSample() const = 0;
        virtual float getSampleRate() const = 0;
        
        inline port_delegate_t const& portDelegate() const { return m_PortDelegate; }
        
    protected:
        inline void notifyOnPortConfigurationChanged()
        {
            m_PortDelegate.call(&PortDelegate::onPortConfigurationChanged);
        }

    protected:
        port_delegate_t m_PortDelegate;
    };
    
    class InputPort
        : public virtual Port
        , public PortDelegate
    {
    public:
        friend class OutputPort;
        friend class Graph;
        
        InputPort();
        ~InputPort();
        
        virtual intrusive_ptr<Sink> getSink() const = 0;
        
        
        SampleType getSampleType() const;
        Sample getSample() const;
        float getSampleRate() const;
        
        inline intrusive_ptr<OutputPort> const& getIncomingPort() const { return m_IncomingPort; }
        
        template <typename T>
        intrusive_ptr<T> getIncoming() const { return dynamic_pointer_cast<T>(getIncomingPort()); }
        
        
    protected:
        system::error_code setIncomingPort(intrusive_ptr<OutputPort> const &op);
        void onPortConfigurationChanged();
        virtual bool acceptsPort(intrusive_ptr<OutputPort> const &op) const = 0;
        
        
    protected:
        intrusive_ptr<OutputPort> m_IncomingPort;
    };
    
    class OutputPort
        : public virtual Port
    {
    public:
        friend class Graph;
        
        OutputPort();
        ~OutputPort();
        
        virtual intrusive_ptr<Source> getSource() const = 0;
        
        inline unordered_set< intrusive_ptr<InputPort> > const& getOutgoingPorts() const { return m_OutgoingPorts; }
        
    protected:
        system::error_code addOutgoingPort(intrusive_ptr<InputPort> const &ip);
        system::error_code removeOutgoingPort(intrusive_ptr<InputPort> const &ip);
        void clearOutgoingPorts();
        
    protected:
        unordered_set< intrusive_ptr<InputPort> > m_OutgoingPorts;
    };
    
    class VideoPort
        : public virtual Port
    {
    public:
        VideoPort();
        ~VideoPort();
        
        virtual Vec2i getVideoSampleSize() const = 0;
    };
    
    class VideoInputPort
        : public InputPort
        , public VideoPort
    {
    public:
        VideoInputPort();
        ~VideoInputPort();
        
        Vec2i getVideoSampleSize() const;
    };
    
    class VideoOutputPort
        : public OutputPort
        , public VideoPort
    {
    public:
        VideoOutputPort();
        ~VideoOutputPort();
    };

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_PORT_H
