#ifndef UQUAD_MEDIA_PLAYOUT_H
#define UQUAD_MEDIA_PLAYOUT_H

#include "Config.h"
#include "Consumer.h"
#include "../base/PixelSampleBuffer.h"
#include "../base/Delegate.h"

namespace uquad
{
namespace media
{
    class PlayoutDelegate
	{
	public:
		PlayoutDelegate() = default;
		virtual ~PlayoutDelegate() = default;
        
        virtual void onPlayoutVideoResized(Vec2i const &size) {}
        virtual void onPlayoutVideoFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer) {}
	};
    
    
    class Playout
    	: public Consumer
    {
    public:
        typedef base::Delegate<PlayoutDelegate> playout_delegate_t;
        
        class VideoInputPort
            : public media::VideoInputPort
        {
        public:
            VideoInputPort(Playout &po);
            ~VideoInputPort();
            
            intrusive_ptr<Sink> getSink() const;
            
            bool acceptsPort(intrusive_ptr<OutputPort> const &op) const;
        
        private:
            Playout &m_Playout;
        };
        
        Playout();
    	~Playout();
        
        bool isValid() const;
        
        system::error_code prepare(asio::yield_context &yctx);
        void unprepare(asio::yield_context &yctx);
        
        system::error_code run(asio::yield_context &yctx);
        void rest(asio::yield_context &yctx);
        
        std::size_t numInputPorts() const;
        intrusive_ptr<InputPort> getInputPort(std::size_t index) const;
        
        inline playout_delegate_t const& playoutDelegate() { return m_PlayoutDelegate; }
        
    protected:
        
        inline void notifyOnPlayoutVideoResized(Vec2i const &size)
        {
            m_PlayoutDelegate.call(&PlayoutDelegate::onPlayoutVideoResized, size);
        }
        
        inline void notifyOnPlayoutVideoFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer)
        {
            m_PlayoutDelegate.call(&PlayoutDelegate::onPlayoutVideoFrame, buffer);
        }
        
    protected:
        mutable VideoInputPort m_VideoInputPort;
        playout_delegate_t m_PlayoutDelegate;
        
    };

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_PLAYOUT_H
