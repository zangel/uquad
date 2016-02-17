#ifndef UQUAD_MEDIA_RAW_WRITER_H
#define UQUAD_MEDIA_RAW_WRITER_H

#include "Config.h"
#include "Writer.h"
#include "VideoSampleFormat.h"
#include "../base/SampleBuffer.h"
#include "../base/YieldEvent.h"



namespace uquad
{
namespace media
{
    struct RawContainerHeader;
    
	class RawWriter
		: public Writer
	{
	public:
		class Registry
			: public Writer::Registry
		{
		public:
			Registry();
			~Registry();

            eContainerFormat getContainerFormat() const;
			intrusive_ptr<media::Component> createComponent() const;
		};
        
        class VideoInputPort
            : public media::VideoInputPort
        {
        public:
            VideoInputPort(Writer &w);
            ~VideoInputPort();
            
            intrusive_ptr<Sink> getSink() const;
            
            bool acceptsPort(intrusive_ptr<OutputPort> const &op) const;
        
        private:
            Writer &m_Writer;
        };
        
        RawWriter();
		~RawWriter();

        bool isValid() const;
        
        system::error_code prepare(asio::yield_context &yctx);
        void unprepare(asio::yield_context &yctx);
        
        system::error_code resume(asio::yield_context &yctx);
        void pause(asio::yield_context &yctx);
        
        system::error_code run(asio::yield_context &yctx);
        void rest(asio::yield_context &yctx);
        
        std::size_t numInputPorts() const;
        intrusive_ptr<InputPort> getInputPort(std::size_t index) const;
        
        eContainerFormat getContainerFormat() const;
        VideoSampleFormat const& getVideoSampleFormat() const;
        
        system::error_code setVideoSampleFormat(VideoSampleFormat const &fmt);
        
        intrusive_ptr<base::IOStream> getStream() const { return m_Stream; }
        system::error_code setStream(intrusive_ptr<base::IOStream> const &stm);
    
    private:
        system::error_code writeVideoBuffer();
        void asyncWriteVideoBuffer();
        void handleAsyncWriteVideoBufferSize(system::error_code e, std::size_t n);
        void handleAsyncWriteVideoBufferData(system::error_code e, std::size_t n, std::size_t offset);
        
        
	private:
        mutable VideoInputPort m_VideoInputPort;
        intrusive_ptr<base::IOStream> m_Stream;
        intrusive_ptr<base::IOAsyncStream> m_AsyncStream;
        
        intrusive_ptr<base::YieldEvent> m_YieldEvent;
        
        VideoSampleFormat m_VideoSampleFormat;
        
        uint32_t m_CurrentVideoBufferSize;
        intrusive_ptr<base::SampleBuffer> m_CurrentVideoBuffer;
	};

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_RAW_WRITER_H
