#ifndef UQUAD_MEDIA_OMX_VIDEO_ENCODER_H
#define UQUAD_MEDIA_OMX_VIDEO_ENCODER_H

#include "../Config.h"
#include "../VideoEncoder.h"
#include "../../base/YieldEvent.h"
#include <OMX_Core.h>

namespace uquad
{
namespace media
{
namespace omx
{
	class VideoEncoder
        : public media::VideoEncoder
    {
	public:
	    class Registry
            : public media::VideoEncoder::Registry
        {
        public:
            Registry(std::string const &n, eVideoCodecType vct, std::size_t oci);
            ~Registry();

            eVideoCodecType getVideoCodecType() const;
            intrusive_ptr<media::Component> createComponent() const;

            eVideoCodecType videoCodecType;
            std::size_t ocxIndex;
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

        class VideoOutputBuffer
            : public base::SampleBuffer
        {
        public:
            VideoOutputBuffer(VideoEncoder &ve, OMX_BUFFERHEADERTYPE *bh);
            ~VideoOutputBuffer();

            bool isValid() const;
            void* data();
            std::size_t size() const;

            system::error_code lock();
            system::error_code unlock();

            inline OMX_BUFFERHEADERTYPE* getOMXBuffer() { return m_BufferHeader; }

        private:
            VideoEncoder &m_VideoEncoder;
            OMX_BUFFERHEADERTYPE *m_BufferHeader;
            atomic<uint32_t> m_LockCounter;
        };

        static const std::size_t s_IOPortsBufferMaxCapacity = 64;
        typedef lockfree::spsc_queue
        <
            OMX_BUFFERHEADERTYPE*,
            lockfree::capacity<s_IOPortsBufferMaxCapacity>
        > IOPortBufferQueue;

        typedef accumulators::accumulator_set
        <
            float,
            accumulators::stats
            <
                accumulators::tag::min,
                accumulators::tag::max,
                accumulators::tag::mean,
                accumulators::tag::variance
            >
        > float_stats_t;


        VideoEncoder(Registry const &r);
        ~VideoEncoder();

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

    private:
        bool setupInputPort();
        bool setupOutputPort();
        bool setupProfileAndLevel();
        bool setupBitrate();
        bool setupFramerate();

        static OMX_ERRORTYPE _omxEventHandler(OMX_HANDLETYPE hc, OMX_PTR ve, OMX_EVENTTYPE evt, OMX_U32 nd1, OMX_U32 nd2, OMX_PTR evtd);
        static OMX_ERRORTYPE _omxEmptyBufferDone(OMX_HANDLETYPE hc, OMX_PTR ve, OMX_BUFFERHEADERTYPE *buffer);
        static OMX_ERRORTYPE _omxFillBufferDone(OMX_HANDLETYPE hc, OMX_PTR ve, OMX_BUFFERHEADERTYPE* buffer);

        OMX_ERRORTYPE omxEventHandler(OMX_EVENTTYPE evt, OMX_U32 nd1, OMX_U32 nd2, OMX_PTR evtd);
        OMX_ERRORTYPE omxEmptyBufferDone(OMX_BUFFERHEADERTYPE *buffer);
        OMX_ERRORTYPE omxFillBufferDone(OMX_BUFFERHEADERTYPE* buffer);

        void omxAllocBuffers();
        void omxFreeBuffers();

        void returnOMXOutputBuffer(OMX_BUFFERHEADERTYPE *bh);


    private:
        Registry const &m_Registry;

        OMX_HANDLETYPE m_OMXHandle;
        OMX_INDEXTYPE m_ExtStoreMetaDataInBuffersIndex;
        OMX_INDEXTYPE m_ExtPrependSPSPPSToIDRFramesIndex;
        mutable VideoInputPort m_VideoInputPort;
        mutable VideoOutputPort m_VideoOutputPort;

        intrusive_ptr<base::SampleBuffer> m_pVideoInputBuffer;
        intrusive_ptr<base::SampleBuffer> m_pVideoOutputBuffer;

        eCodecProfile m_Profile;
        int m_VideoBitRate;

        Vec2i m_VideoSampleSize;
        float m_VideoSampleRate;

        float m_VideoSampleTime;

        atomic<OMX_STATETYPE> m_OMXState;

        std::size_t m_OMXInputPortBufferSize;
        std::vector<OMX_BUFFERHEADERTYPE*> m_OMXInputPortBuffers;
        IOPortBufferQueue m_OMXInputPortBuffersQueue;

        std::size_t m_OMXOutputPortBufferSize;
        std::vector<OMX_BUFFERHEADERTYPE*> m_OMXOutputPortBuffers;
        IOPortBufferQueue m_OMXOutputPortBuffersQueue;

        intrusive_ptr<base::YieldEvent> m_EBDYieldEvent;
        intrusive_ptr<base::YieldEvent> m_StateYieldEvent;

        //chrono::steady_clock::time_point m_LastStatsTime;
        //float_stats_t m_EncodeTimeStats;
        //float_stats_t m_BandwidthStats;
        //atomic<std::size_t> m_BytesEncoded;
    };

} //namespace omx
} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_OMX_VIDEO_ENCODER_H
