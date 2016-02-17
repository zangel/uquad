#ifndef UQUAD_MEDIA_FFMPEG_VIDEO_ENCODER_H
#define UQUAD_MEDIA_FFMPEG_VIDEO_ENCODER_H

#include "../Config.h"
#include "../VideoEncoder.h"
#include "FFMpeg.h"
#include "AVPacketSampleBuffer.h"

namespace uquad
{
namespace media
{
namespace ffmpeg
{
	class VideoEncoder
		: public media::VideoEncoder
	{
	public:
		class Registry
			: public media::VideoEncoder::Registry
		{
		public:
			Registry(std::string const &n, ::AVCodec *c);
			~Registry();

            eVideoCodecType getVideoCodecType() const;
			intrusive_ptr<media::Component> createComponent() const;
            
            ::AVCodec *m_FFMpegCodec;
		};
        
        class VideoInputPort
            : public media::VideoInputPort
        {
        public:
            VideoInputPort(VideoEncoder &ve);
            ~VideoInputPort();
            
            intrusive_ptr<Sink> getSink() const;
            
            bool acceptsPort(intrusive_ptr<OutputPort> const &op) const;
            
            
        private:
            VideoEncoder &m_VideoEncoder;
        };
        
        class VideoOutputPort
            : public media::VideoOutputPort
        {
        public:
            VideoOutputPort(VideoEncoder &ve);
            ~VideoOutputPort();
            
            intrusive_ptr<Source> getSource() const;
            
            SampleType getSampleType() const;
            Sample getSample() const;
            float getSampleRate() const;
            Vec2i getVideoSampleSize() const;
            
        private:
            VideoEncoder &m_VideoEncoder;
        };
        
        VideoEncoder(Registry const &r);
		~VideoEncoder();

		bool isValid() const;
        
        system::error_code prepare(asio::yield_context &yctx);
        void unprepare();
        
        system::error_code run(asio::yield_context &yctx);
        void rest();
        
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
	private:
        ::AVCodec *m_FFMpegCodec;
        ::AVCodecContext *m_FFMpegCodecCtx;
        mutable VideoInputPort m_VideoInputPort;
        mutable VideoOutputPort m_VideoOutputPort;
        mutable AVPacketSampleBuffer m_VideoOutputBuffer;
        intrusive_ptr<base::SampleBuffer> m_pVideoOutputBuffer;
        ::SwsContext *m_VideoFrameConverter;
        ::AVFrame m_VideoFrameTransform;
        float m_VideoSampleTime;
        eCodecProfile m_Profile;
        int m_VideoBitRate;
	};

} //namespace ffmpeg
} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_FFMPEG_VIDEO_ENCODER_H
