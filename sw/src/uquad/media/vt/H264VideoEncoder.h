#ifndef UQUAD_MEDIA_VT_H264_VIDEO_ENCODER_H
#define UQUAD_MEDIA_VT_H264_VIDEO_ENCODER_H

#include "../Config.h"
#include "../VideoEncoder.h"
#include <VideoToolbox/VideoToolbox.h>

namespace uquad
{
namespace media
{
namespace vt
{
	class H264VideoEncoder
		: public media::VideoEncoder
	{
	public:
		class Registry
			: public media::VideoEncoder::Registry
		{
		public:
			Registry(std::string const &n, CFDictionaryRef es);
			~Registry();

            eVideoCodecType getVideoCodecType() const;
			intrusive_ptr<media::Component> createComponent() const;
            
            CFDictionaryRef encoderSpec;
		};
        
        class VideoInputPort
            : public media::VideoInputPort
        {
        public:
            VideoInputPort(H264VideoEncoder &ve);
            ~VideoInputPort();
            
            intrusive_ptr<Sink> getSink() const;
            
            bool acceptsPort(intrusive_ptr<OutputPort> const &op) const;
            
            
        private:
            H264VideoEncoder &m_VideoEncoder;
        };
        
        class VideoOutputPort
            : public media::VideoOutputPort
        {
        public:
            VideoOutputPort(H264VideoEncoder &ve);
            ~VideoOutputPort();
            
            intrusive_ptr<Source> getSource() const;
            
            SampleType getSampleType() const;
            Sample getSample() const;
            float getSampleRate() const;
            Vec2i getVideoSampleSize() const;
            
        private:
            H264VideoEncoder &m_VideoEncoder;
        };
        
        class VideoOutputBuffer
            : public base::SampleBuffer
        {
        public:
            VideoOutputBuffer(CMSampleBufferRef cmsb);
            ~VideoOutputBuffer();
            
            bool isValid() const;
            void* data();
            std::size_t size() const;
            
            system::error_code lock();
            system::error_code unlock();
            
        private:
            void init();
            
            
        private:
            CMSampleBufferRef m_CMSampleBuffer;
            std::vector<uint8_t> m_Data;
        };
        
        H264VideoEncoder(Registry const &r);
		~H264VideoEncoder();

		bool isValid() const;
        
        system::error_code prepare(asio::yield_context &yctx);
        void unprepare(asio::yield_context &yctx);
        
        system::error_code resume(asio::yield_context &yctx);
        void pause(asio::yield_context &yctx);
        
        system::error_code run(asio::yield_context &yctx);
        void rest(asio::yield_context &yctx);
        
        std::size_t numInputPorts() const;
        intrusive_ptr<InputPort> getInputPort(std::size_t index) const;
        
        std::size_t numOutputPorts() const;
        intrusive_ptr<OutputPort> getOutputPort(std::size_t index) const;
        
        eCodecProfile getProfile() const;
        bool setProfile(eCodecProfile profile);
        
        int getBitRate() const;
        bool setBitRate(int br);
        
        eVideoCodecType getVideoCodecType() const;
        
        SampleType getOutputPortSampleType() const;
        Sample getOutputPortSample() const;
        float getOutputPortSampleRate() const;
        Vec2i getOutputPortVideoSampleSize() const;

        bool acceptsInputPortPixelFormat(base::ePixelFormat fmt) const;
        
	private:
        void vtCompressionOutputCallback(void *frame, OSStatus status, VTEncodeInfoFlags iflags, CMSampleBufferRef sb);
        static void _vtCompressionOutputCallback(void *_this, void *frame, OSStatus status, VTEncodeInfoFlags iflags, CMSampleBufferRef sb);

	private:
        Registry const &m_Registry;
        mutable VideoInputPort m_VideoInputPort;
        mutable VideoOutputPort m_VideoOutputPort;
        intrusive_ptr<base::SampleBuffer> m_pVideoOutputBuffer;
        
        VTCompressionSessionRef m_CompressionSession;
        
        bool m_ForceVideoKeyFrame;
        
        Vec2i m_VideoSampleSize;
        float m_VideoSampleRate;
        
        float m_VideoSampleTime;
        
        eCodecProfile m_Profile;
        int m_VideoBitRate;
	};

} //namespace vt
} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_VT_H264_VIDEO_ENCODER_H
