#ifndef UQUAD_MEDIA_VT_H264_VIDEO_DECODER_H
#define UQUAD_MEDIA_VT_H264_VIDEO_DECODER_H

#include "../Config.h"
#include "../VideoDecoder.h"
#include "VT.h"

namespace uquad
{
namespace media
{
namespace vt
{
	class H264VideoDecoder
		: public media::VideoDecoder
	{
	public:
		class Registry
			: public media::VideoDecoder::Registry
		{
		public:
			Registry(std::string const &n);
			~Registry();

            eVideoCodecType getVideoCodecType() const;
			intrusive_ptr<media::Component> createComponent() const;
		};
        
        class VideoInputPort
            : public media::VideoInputPort
        {
        public:
            VideoInputPort(H264VideoDecoder &vd);
            ~VideoInputPort();
            
            intrusive_ptr<Sink> getSink() const;
            
            bool acceptsPort(intrusive_ptr<OutputPort> const &op) const;
            
            
        private:
            H264VideoDecoder &m_VideoDecoder;
        };
        
        class VideoOutputPort
            : public media::VideoOutputPort
        {
        public:
            VideoOutputPort(H264VideoDecoder &vd);
            ~VideoOutputPort();
            
            intrusive_ptr<Source> getSource() const;
            
            SampleType getSampleType() const;
            Sample getSample() const;
            float getSampleRate() const;
            Vec2i getVideoSampleSize() const;
            
        private:
            H264VideoDecoder &m_VideoDecoder;
        };
        
		H264VideoDecoder(Registry const &r);
		~H264VideoDecoder();

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
        
        bool acceptsInputPortVideoCodecType(eVideoCodecType ct) const;
        
        SampleType getOutputPortSampleType() const;
        Sample getOutputPortSample() const;
        float getOutputPortSampleRate() const;
        Vec2i getOutputPortVideoSampleSize() const;
        
	private:
        void vtDecodeCallback(void *frame, OSStatus status, VTDecodeInfoFlags iflags, CVImageBufferRef ib, CMTime pts, CMTime pd);
        static void _vtDecodeCallback(void *_this, void *frame, OSStatus status, VTDecodeInfoFlags iflags, CVImageBufferRef ib, CMTime pts, CMTime pd);
        
    
	private:
        mutable VideoInputPort m_VideoInputPort;
        mutable VideoOutputPort m_VideoOutputPort;
        base::ePixelFormat m_VideoOutputFormat;
        
        VTDecompressionSessionRef m_VTDSession;
        CMVideoFormatDescriptionRef m_CMFormatDesc;
        
        
        intrusive_ptr<base::SampleBuffer> m_pVideoOutputBuffer;
        
        eCodecProfile m_Profile;
        int m_VideoBitRate;
        
        Vec2i m_VideoSampleSize;
        float m_VideoSampleRate;

        float m_VideoSampleTime;
	};

} //namespace ffmpeg
} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_VT_H264_VIDEO_DECODER_H
