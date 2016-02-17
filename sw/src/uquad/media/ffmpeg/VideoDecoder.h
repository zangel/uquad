#ifndef UQUAD_MEDIA_FFMPEG_VIDEO_DECODER_H
#define UQUAD_MEDIA_FFMPEG_VIDEO_DECODER_H

#include "../Config.h"
#include "../VideoDecoder.h"
#include "AVFramePixelSampleBuffer.h"
#include "FFMpeg.h"

namespace uquad
{
namespace media
{
namespace ffmpeg
{
	class VideoDecoder
		: public media::VideoDecoder
	{
	public:
		class Registry
			: public media::VideoDecoder::Registry
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
            VideoInputPort(VideoDecoder &vd);
            ~VideoInputPort();
            
            intrusive_ptr<Sink> getSink() const;
            
            bool acceptsPort(intrusive_ptr<OutputPort> const &op) const;
            
            
        private:
            VideoDecoder &m_VideoDecoder;
        };
        
        class VideoOutputPort
            : public media::VideoOutputPort
        {
        public:
            VideoOutputPort(VideoDecoder &vd);
            ~VideoOutputPort();
            
            intrusive_ptr<Source> getSource() const;
            
            SampleType getSampleType() const;
            Sample getSample() const;
            float getSampleRate() const;
            Vec2i getVideoSampleSize() const;
            
        private:
            VideoDecoder &m_VideoDecoder;
        };
        
		VideoDecoder(Registry const &r);
		~VideoDecoder();

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
        
        bool acceptsInputPortVideoCodecType(eVideoCodecType ct) const;
        
        SampleType getOutputPortSampleType() const;
        Sample getOutputPortSample() const;
        float getOutputPortSampleRate() const;
        Vec2i getOutputPortVideoSampleSize() const;
        
	private:
	private:
        ::AVCodec *m_FFMpegCodec;
        ::AVCodecContext *m_FFMpegCodecCtx;
        mutable VideoInputPort m_VideoInputPort;
        mutable VideoOutputPort m_VideoOutputPort;
        base::ePixelFormat m_VideoOutputFormat;
        mutable AVFramePixelSampleBuffer m_VideoOutputBuffer;
        intrusive_ptr<base::SampleBuffer> m_pVideoOutputBuffer;
        ::AVPixelFormat m_VideoFrameConverterSrcFormat;
        ::SwsContext *m_VideoFrameConverter;
        float m_VideoSampleTime;
        eCodecProfile m_Profile;
        int m_VideoBitRate;
	};

} //namespace ffmpeg
} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_FFMPEG_VIDEO_DECODER_H
