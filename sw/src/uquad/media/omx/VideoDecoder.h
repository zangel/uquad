#ifndef UQUAD_MEDIA_OMX_VIDEO_DECODER_H
#define UQUAD_MEDIA_OMX_VIDEO_DECODER_H

#include "../Config.h"
#include "../VideoDecoder.h"
#include <OMX_Core.h>

namespace uquad
{
namespace media
{
namespace omx
{
    class VideoDecoder
        : public media::VideoDecoder
    {
    public:
        class Registry
            : public media::VideoDecoder::Registry
        {
        public:
            Registry(std::string const &n, eVideoCodecType vct, std::size_t oci);
            ~Registry();

            intrusive_ptr<media::Component> createComponent() const;

            eVideoCodecType getVideoCodecType() const;

            eVideoCodecType videoCodecType;
            std::size_t ocxIndex;
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
        static OMX_ERRORTYPE _omxEventHandler(OMX_HANDLETYPE hc, OMX_PTR vd, OMX_EVENTTYPE evt, OMX_U32 nd1, OMX_U32 nd2, OMX_PTR evtd);
        static OMX_ERRORTYPE _omxEmptyBufferDone(OMX_HANDLETYPE hc, OMX_PTR vd, OMX_BUFFERHEADERTYPE *buffer);
        static OMX_ERRORTYPE _omxFillBufferDone(OMX_HANDLETYPE hc, OMX_PTR vd, OMX_BUFFERHEADERTYPE* buffer);

        OMX_ERRORTYPE omxEventHandler(OMX_EVENTTYPE evt, OMX_U32 nd1, OMX_U32 nd2, OMX_PTR evtd);
        OMX_ERRORTYPE omxEmptyBufferDone(OMX_BUFFERHEADERTYPE *buffer);
        OMX_ERRORTYPE omxFillBufferDone(OMX_BUFFERHEADERTYPE* buffer);

    private:
        Registry const &m_Registry;
        OMX_HANDLETYPE m_OMXHandle;
        mutable VideoInputPort m_VideoInputPort;
        mutable VideoOutputPort m_VideoOutputPort;

        eCodecProfile m_Profile;
        int m_VideoBitRate;

        Vec2i m_VideoSampleSize;
        float m_VideoSampleRate;

        float m_VideoSampleTime;
    };

} //namespace omx
} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_OMX_VIDEO_DECODER_H
