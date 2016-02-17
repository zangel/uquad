#ifndef UQUAD_MEDIA_RAW_READER_H
#define UQUAD_MEDIA_RAW_READER_H

#include "Config.h"
#include "Reader.h"
#include "../base/SampleBuffer.h"
#include "../base/YieldEvent.h"

namespace uquad
{
namespace media
{
    struct RawContainerHeader;

	class RawReader
		: public Reader
	{
	public:
		class Registry
			: public Reader::Registry
		{
		public:
			Registry();
			~Registry();

            eContainerFormat getContainerFormat() const;
			intrusive_ptr<media::Component> createComponent() const;
		};
        
        class VideoOutputPort
            : public media::VideoOutputPort
        {
        public:
            VideoOutputPort(RawReader &rrdr);
            ~VideoOutputPort();
            
            intrusive_ptr<Source> getSource() const;
            SampleType getSampleType() const;
            Sample getSample() const;
            float getSampleRate() const;
            Vec2i getVideoSampleSize() const;
            
        private:
            RawReader &m_Reader;
        };
        
        class VideoOutputBuffer
            : public base::SampleBuffer
        {
        public:
            friend class RawReader;
            
            VideoOutputBuffer();
            ~VideoOutputBuffer();
            
            bool isValid() const;
            void* data();
            std::size_t size() const;
            
            system::error_code lock();
            system::error_code unlock();
            
        private:
            std::vector<uint8_t> m_Data;
            uint32_t m_Size;
        };
        
        
        RawReader();
		~RawReader();

        bool isValid() const;
        
        system::error_code prepare(asio::yield_context &yctx);
        void unprepare(asio::yield_context &yctx);
        
        system::error_code resume(asio::yield_context &yctx);
        void pause(asio::yield_context &yctx);
        
        system::error_code run(asio::yield_context &yctx);
        void rest(asio::yield_context &yctx);
        
        std::size_t numOutputPorts() const;
        intrusive_ptr<OutputPort> getOutputPort(std::size_t index) const;
        
        eContainerFormat getContainerFormat() const;
        VideoSampleFormat const& getVideoSampleFormat() const;
        
        system::error_code setVideoSampleFormat(VideoSampleFormat const &fmt);
        
        intrusive_ptr<base::IOStream> getStream() const { return m_Stream; }
        system::error_code setStream(intrusive_ptr<base::IOStream> const &stm);
        
        SampleType getOutputPortSampleType() const;
        Sample getOutputPortSample() const;
        float getOutputPortSampleRate() const;
        Vec2i getOutputPortVideoSampleSize() const;
        
    private:
        void readVideoBuffer();
        void asyncReadVideoBuffer();
        void handleAsyncReadVideoBufferSize(system::error_code e, std::size_t n);
        void handleAsyncReadVideoBufferData(system::error_code e, std::size_t n, std::size_t offset);
        
        
	private:
        mutable VideoOutputPort m_VideoOutputPort;
        mutable VideoOutputBuffer m_VideoOutputBuffer;
        
        intrusive_ptr<base::SampleBuffer> m_pVideoOutputBuffer;
        
        intrusive_ptr<base::IOStream> m_Stream;
        intrusive_ptr<base::IOAsyncStream> m_AsyncStream;
        
        VideoSampleFormat m_VideoSampleFormat;
        
        float m_VideoSampleTime;
        
        intrusive_ptr<base::YieldEvent> m_YieldEvent;
	};

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_RAW_READER_H
