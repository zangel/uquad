#ifndef UQUAD_HAL_QCOM_CAMERA_H
#define UQUAD_HAL_QCOM_CAMERA_H

#include "../Camera.h"

extern "C"
{

	#include <camera/mm_camera_interface.h>
}


namespace uquad
{
namespace hal
{
    class Camera::Impl
    {
    public:
    	typedef union
    	{
    		cam_ctrl_dimension_t dim;
    		uint8_t stg[440];
    	} cam_ctrl_dimension_wrapper_t;

    	class Stream
    	{
    	public:
    		friend class Camera::Impl;

    		Stream(Camera::Impl &impl);
    		virtual ~Stream();

    		inline uint32_t getStreamId()  const { return m_StreamId; }

    		void setFormat(uint32_t fmt);
    		void setSize(Vec2i const &size);

    		virtual system::error_code create() = 0;
    		inline bool isCreated() const { return m_StreamId > 0; }
    		virtual system::error_code destroy() = 0;


    		virtual system::error_code start() = 0;
    		inline bool isStarted() const { return m_bStarted; }
    		virtual system::error_code stop() = 0;

    		virtual int32_t getBuf(mm_camera_frame_len_offset *foi, uint8_t nb, uint8_t *irf, mm_camera_buf_def_t  *bufs) = 0;
    		virtual int32_t putBuf(uint8_t nb, mm_camera_buf_def_t *bufs) = 0;

    	protected:
    		virtual void callback(mm_camera_super_buf_t *bufs);

    	private:
    		static void _callback(mm_camera_super_buf_t *bufs, void *ud);

    	protected:
    		Impl &m_Impl;
    		Vec2i m_Size;
    		uint32_t m_Format;
    		uint32_t m_StreamId;
    		bool m_bStarted;
    	};

    	class PreviewStream
    		: public Stream
    	{
    	public:
    		friend class Camera::Impl;

    		PreviewStream(Camera::Impl &impl);
    		~PreviewStream();

    		system::error_code create();
			system::error_code destroy();

			system::error_code start();
			system::error_code stop();

			int32_t getBuf(mm_camera_frame_len_offset *foi, uint8_t nb, uint8_t *irf, mm_camera_buf_def_t  *bufs);
			int32_t putBuf(uint8_t nb, mm_camera_buf_def_t *bufs);


    	protected:
    		void callback(mm_camera_super_buf_t *bufs);
    	};

    	static uint32_t numCameras();

    	Impl(Camera &camera, uint32_t camidx);
        ~Impl();

        bool isOpen() const;
        system::error_code open();
        system::error_code close();

        std::vector<base::ePixelFormat> const& getSupportedPreviewFormats() const;
        std::vector<base::ePixelFormat> const& getSupportedPictureFormats() const;

        std::vector<Vec2i> const& getSupportedPreviewSizes() const;
        std::vector<Vec2i> const& getSupportedPictureSizes() const;

        float getPreviewFPS() const;
        void setPreviewFPS(float fr);

        core::ePixelFormat getPreviewFormat() const;
        void setPreviewFormat(core::ePixelFormat fmt);

        Vec2i const& getPreviewSize() const;
        void setPreviewSize(Vec2i const &sz);

        core::ePixelFormat getPictureFormat() const;
        void setPictureFormat(core::ePixelFormat fmt);

        Vec2i const& getPictureSize() const;
        void setPictureSize(Vec2i const &sz);

        system::error_code startPreview();
        bool isPreviewStarted() const;
        system::error_code stopPreview();


    private:
        system::error_code init();
        system::error_code setup();
        system::error_code createPreviewStream();



    private:
        static int32_t _getBuf(uint32_t camh, uint32_t cid, uint32_t sid, void *ud, mm_camera_frame_len_offset *foi, uint8_t nb, uint8_t *irf, mm_camera_buf_def_t *bufs);
        static int32_t _putBuf(uint32_t camh, uint32_t cid, uint32_t sid, void *ud, uint8_t nb, mm_camera_buf_def_t *bufs);
        static void _processHalEvent(uint32_t camh, mm_camera_event_t *e, void *ud);

        int32_t getBuf(uint32_t camh, uint32_t cid, uint32_t sid, mm_camera_frame_len_offset *foi, uint8_t nb, uint8_t *irf, mm_camera_buf_def_t *bufs);
        int32_t putBuf(uint32_t camh, uint32_t cid, uint32_t sid, uint8_t nb, mm_camera_buf_def_t *bufs);
        void processHalEvent(uint32_t camh, mm_camera_event_t *e);
        void processChannelEvent(mm_camera_ch_event_t *e);
        void processCtrlEvent(mm_camera_ctrl_event_t *e);
        void processStatsEvent(mm_camera_stats_event_t *e);
        void processInfoEvent(mm_camera_info_event_t *e);

        void processPreviewChannelEvent(mm_camera_ch_event_type_t che);
        void processRdiChannelEvent(mm_camera_ch_event_type_t che);
        void processRecordChannelEvent(mm_camera_ch_event_type_t che);
        void processSnapshotChannelEvent(mm_camera_ch_event_type_t che);

        system::error_code applyDimension();


    private:
        Camera &m_Camera;
        uint32_t m_CameraIdx;
        mm_camear_mem_vtbl_t m_CameraMemVTbl;
        mm_camera_vtbl_t *m_pCameraVTbl;
        uint32_t m_CameraMode;
        uint32_t m_ChannelId;

        cam_ctrl_dimension_wrapper_t m_CameraDimension;

        std::vector<core::ePixelFormat> m_SupportedPreviewFormats;
        std::vector<core::ePixelFormat> m_SupportedPictureFormats;

        std::vector<Vec2i> m_SupportedPreviewSizes;
        std::vector<Vec2i> m_SupportedPictureSizes;

        Vec2i m_MaxPictureSize;

        Vec2i m_PreviewSize;
        Vec2i m_PictureSize;
        float m_PreviewFPS;
        core::ePixelFormat m_PreviewFormat;
        core::ePixelFormat m_PictureFormat;

        Stream *m_PreviewStream;

        bool m_bPreviewStarted;

    };

} //namespace hal
} //namespace uquad


#endif //UQUAD_HAL_QCOM_CAMERA_H
