#ifndef UQUAD_MEDIA_FFMPEG_WRITER_H
#define UQUAD_MEDIA_FFMPEG_WRITER_H

#include "../Config.h"
#include "../Writer.h"
#include "FFMpeg.h"
#include <fstream>

namespace uquad
{
namespace media
{
namespace ffmpeg
{
	class Writer
		: public media::Writer
	{
	public:
		class Registry
			: public media::Writer::Registry
		{
		public:
			Registry(std::string const &n, ::AVOutputFormat *f);
			~Registry();

            eContainerFormat getContainerFormat() const;
			intrusive_ptr<media::Component> createComponent() const;
            
            ::AVOutputFormat *m_FFMpegOutputFormat;
            eContainerFormat m_ContainerFormat;
		};
        
        class VideoInputPort
            : public media::VideoInputPort
        {
        public:
            VideoInputPort(Writer &w);
            ~VideoInputPort();
            
            intrusive_ptr<Sink> getSink() const;
            
            bool accepts(intrusive_ptr<OutputPort> const &op) const;
            
            
        private:
            Writer &m_Writer;
        };
        
        

		Writer(Registry const &r);
		~Writer();

		bool isValid() const;
        
        bool prepareExecuting();
        void doneExecuting();
        
        bool execute();
        void postExecute();
        
        std::size_t numInputPorts() const;
        intrusive_ptr<InputPort> getInputPort(std::size_t index) const;
        
        eContainerFormat getContainerFormat() const;
        bool acceptsVideoSampleKind(eVideoSampleKind kind) const;
        
    private:
        static int _ffmpegReadPacket(void *opaque, uint8_t *buf, int buf_size);
        static int _ffmpegWritePacket(void *opaque, uint8_t *buf, int buf_size);
        static int64_t _ffmpegSeek(void *opaque, int64_t offset, int whence);
        
	private:
        ::AVOutputFormat *m_FFMpegOutputFormat;
        eContainerFormat m_ContainerFormat;
        mutable VideoInputPort m_VideoInputPort;
        ::AVFormatContext *m_FFMpegFormatCtx;
        std::vector<uint8_t> m_FFMpegBuffer;
        ::AVIOContext *m_FFMpegIOContext;
        ::AVStream *m_FFMpegVideoStream;
        float m_VideoSampleTime;
        std::fstream m_TestFile;
        
        
	};

} //namespace ffmpeg
} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_FFMPEG_WRITER_H
