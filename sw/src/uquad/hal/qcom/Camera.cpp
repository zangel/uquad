#include "Camera.h"

namespace uquad
{
namespace hal
{
	namespace
	{
		static Vec2i const s_CameraDefaultPictureSizes [] =
		{
			Vec2i(4000, 3000),
			Vec2i(3200, 2400),
			Vec2i(2592, 1944),
			Vec2i(2048, 1536),
			Vec2i(1920, 1080),
			Vec2i(1600, 1200),
			Vec2i(1280,  768),
			Vec2i(1280,  720),
			Vec2i(1024,	 768),
			Vec2i( 800,	 600),
			Vec2i( 800,	 480),
			Vec2i( 720,	 480),
			Vec2i( 640,	 480),
			Vec2i( 352,	 288),
			Vec2i( 320,	 240),
			Vec2i( 176,	 144)
		};
		static uint32_t const s_CameraDefaultPictureSizesCount = sizeof(s_CameraDefaultPictureSizes)/sizeof(Vec2i);

		static Vec2i const s_CameraDefaultPictureSize = Vec2i(1024, 768);
		static Vec2i const s_CameraDefaultVideoSize = Vec2i(1920, 1080);
		static Vec2i const s_CameraDefaultPreviewSize = Vec2i(640, 480);
		static Vec2i const s_CameraDefaultStreamSize = Vec2i(320, 240);
		static Vec2i const s_CameraDefaultLiveshotSize = Vec2i(2592, 1944);

		static float const s_CameraDefaultPreviewFPS = 30.0f;

		uint32_t findBestSizeMatch(std::vector<Vec2i> const &szs, Vec2i const &sz)
		{
			if(szs.size() == 1) return 0;

			float bestAreaMatch = std::abs(sz(0)*sz(1) - szs[0](0)*szs[0](1));
			uint32_t bestMatch = 0;
			for(uint32_t i = 1; i < szs.size(); ++i)
			{
				float areaMatch = std::abs(sz(0)*sz(1) - szs[i](0)*szs[i](1));
				if(areaMatch < bestAreaMatch)
				{
					bestAreaMatch = areaMatch;
					bestMatch = i;
				}
			}
			return bestMatch;
		}

		struct preview_format_info_t
		{
		   int Hal_format;
		   cam_format_t mm_cam_format;
		   cam_pad_format_t padding;
		   int num_planar;
		};

		static cam_format_t mapCoreToCameraPixelFormat(core::ePixelFormat fmt)
		{
			cam_format_t ret;
			switch(fmt)
			{
			case core::kPFMT_YUV_420_NV21: ret = CAMERA_YUV_420_NV21; break;
			default: ret = CAMERA_YUV_420_NV21; break;
			}
			return ret;
		}
	}

	Camera::Impl::Stream::Stream(Camera::Impl &impl)
		: m_Impl(impl)
		, m_Size()
		, m_Format()
		, m_StreamId(0)
		, m_bStarted(false)
	{
	}

	Camera::Impl::Stream::~Stream()
	{

	}

	void Camera::Impl::Stream::setFormat(uint32_t fmt)
	{
		m_Format = fmt;
	}

	void Camera::Impl::Stream::setSize(Vec2i const &size)
	{
		m_Size = size;
	}

	void Camera::Impl::Stream::callback(mm_camera_super_buf_t *bufs)
	{
	}

	void Camera::Impl::Stream::_callback(mm_camera_super_buf_t *bufs, void *ud)
	{
		reinterpret_cast<Camera::Impl::Stream*>(ud)->callback(bufs);
	}

	Camera::Impl::PreviewStream::PreviewStream(Camera::Impl &impl)
		: Stream(impl)
	{
	}

	Camera::Impl::PreviewStream::~PreviewStream()
	{
	}

	system::error_code Camera::Impl::PreviewStream::create()
	{
		if(m_StreamId > 0)
			return system::errc::make_error_code(system::errc::resource_unavailable_try_again);


		m_StreamId = m_Impl.m_pCameraVTbl->ops->add_stream(m_Impl.m_pCameraVTbl->camera_handle,
			m_Impl.m_ChannelId,
			Camera::Impl::Stream::_callback,
			this,
			m_Impl.m_CameraMode,
			0
		);

		if(m_StreamId == 0)
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}

		if(m_Impl.m_pCameraVTbl->ops->get_parm(m_Impl.m_pCameraVTbl->camera_handle, MM_CAMERA_PARM_PREVIEW_FORMAT, &m_Format) < 0)
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}

		mm_camera_stream_config_t stream_config;
		memset(&stream_config, 0, sizeof(mm_camera_stream_config_t));

		stream_config.fmt.fmt = (cam_format_t)m_Format;
		stream_config.fmt.meta_header = MM_CAMEAR_META_DATA_TYPE_DEF;
		stream_config.fmt.width = m_Size(0);
		stream_config.fmt.height = m_Size(1);
		stream_config.num_of_bufs = 5;
		stream_config.need_stream_on = 1;

		if(m_Impl.m_pCameraVTbl->ops->config_stream(m_Impl.m_pCameraVTbl->camera_handle, m_Impl.m_ChannelId, m_StreamId, &stream_config) < 0)
		{
			m_Impl.m_pCameraVTbl->ops->del_stream(m_Impl.m_pCameraVTbl->camera_handle, m_Impl.m_ChannelId, m_StreamId);
			m_StreamId = 0;
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}

		return system::errc::make_error_code(system::errc::success);
	}

	system::error_code Camera::Impl::PreviewStream::destroy()
	{
		if(m_StreamId == 0)
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}

		if(m_Impl.m_pCameraVTbl->ops->del_stream(m_Impl.m_pCameraVTbl->camera_handle, m_Impl.m_ChannelId, m_StreamId) < 0)
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}

		m_StreamId = 0;

		return system::errc::make_error_code(system::errc::success);
	}

	system::error_code Camera::Impl::PreviewStream::start()
	{
		if(m_bStarted)
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}

		if(m_Impl.m_pCameraVTbl->ops->start_streams(m_Impl.m_pCameraVTbl->camera_handle, m_Impl.m_ChannelId, 1, &m_StreamId) < 0)
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}

		m_bStarted = true;
		return system::errc::make_error_code(system::errc::success);
	}

	system::error_code Camera::Impl::PreviewStream::stop()
	{
		if(!m_bStarted)
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}

		if(m_Impl.m_pCameraVTbl->ops->stop_streams(m_Impl.m_pCameraVTbl->camera_handle, m_Impl.m_ChannelId, 1, &m_StreamId) < 0)
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}

		m_bStarted = false;
		return system::errc::make_error_code(system::errc::success);
	}

	int32_t Camera::Impl::PreviewStream::getBuf(mm_camera_frame_len_offset *foi, uint8_t nb, uint8_t *irf, mm_camera_buf_def_t  *bufs)
	{
		return 0;
	}

	int32_t Camera::Impl::PreviewStream::putBuf(uint8_t nb, mm_camera_buf_def_t *bufs)
	{
		return 0;
	}

	void Camera::Impl::PreviewStream::callback(mm_camera_super_buf_t *bufs)
	{

	}


	uint32_t Camera::Impl::numCameras()
	{
		uint8_t num_cameras = 0;
		mm_camera_info_t *cinfos = ::camera_query(&num_cameras);
		return num_cameras;
	}

	Camera::Impl::Impl(Camera &camera, uint32_t camidx)
		: m_Camera(camera)
		, m_CameraIdx(camidx)
		, m_CameraMemVTbl()
		, m_pCameraVTbl(0)
		, m_CameraMode(CAMERA_MODE_2D | CAMERA_NONZSL_MODE)
		, m_ChannelId(0)
		, m_SupportedPreviewFormats()
		, m_SupportedPictureFormats()
		, m_SupportedPreviewSizes()
		, m_SupportedPictureSizes()
		, m_MaxPictureSize()
		, m_PreviewSize(s_CameraDefaultPreviewSize)
		, m_PictureSize(s_CameraDefaultPictureSize)
		, m_PreviewFPS(s_CameraDefaultPreviewFPS)
		, m_PreviewFormat(core::kPFMT_YUV_420_NV21)
		, m_PictureFormat(core::kPFMT_YUV_420_NV21)
		, m_PreviewStream(0)
		, m_bPreviewStarted(false)
	{
		m_CameraMemVTbl.get_buf = &Camera::Impl::_getBuf;
		m_CameraMemVTbl.put_buf = &Camera::Impl::_putBuf;
		m_CameraMemVTbl.user_data = this;
	}

    Camera::Impl::~Impl()
    {
    }

    bool Camera::Impl::isOpen() const
    {
    	return m_pCameraVTbl != 0;
    }

    system::error_code Camera::Impl::open()
    {
    	if(isOpen())
    		return system::errc::make_error_code(system::errc::already_connected);

    	m_pCameraVTbl = ::camera_open(m_CameraIdx, &m_CameraMemVTbl);
    	if(!m_pCameraVTbl)
    		return system::errc::make_error_code(system::errc::resource_unavailable_try_again);

    	m_pCameraVTbl->ops->sync(m_pCameraVTbl->camera_handle);

    	m_ChannelId = m_pCameraVTbl->ops->ch_acquire(m_pCameraVTbl->camera_handle);
    	if(m_ChannelId<=0)
    	{
    		m_pCameraVTbl->ops->camera_close(m_pCameraVTbl->camera_handle);
    		m_pCameraVTbl = 0;
    		return system::errc::make_error_code(system::errc::resource_unavailable_try_again);
    	}

    	mm_camera_event_type_t evt;
    	for (int i = 0; i < MM_CAMERA_EVT_TYPE_MAX; i++)
    	{
    		if(m_pCameraVTbl->ops->is_event_supported(m_pCameraVTbl->camera_handle, (mm_camera_event_type_t )i))
    		{
    			m_pCameraVTbl->ops->register_event_notify(
    				m_pCameraVTbl->camera_handle,
    				&Camera::Impl::_processHalEvent,
    				(void *)this,
    				(mm_camera_event_type_t) i
    			);
    		}
    	}

    	if(init())
    	{
    		m_pCameraVTbl->ops->ch_release(m_pCameraVTbl->camera_handle, m_ChannelId);
    		m_ChannelId = 0;
    		m_pCameraVTbl->ops->camera_close(m_pCameraVTbl->camera_handle);
			m_pCameraVTbl = 0;
			return system::errc::make_error_code(system::errc::resource_unavailable_try_again);
    	}

    	if(setup())
    	{
    		m_pCameraVTbl->ops->ch_release(m_pCameraVTbl->camera_handle, m_ChannelId);
    		m_ChannelId = 0;
			m_pCameraVTbl->ops->camera_close(m_pCameraVTbl->camera_handle);
			m_pCameraVTbl = 0;
			return system::errc::make_error_code(system::errc::resource_unavailable_try_again);
    	}

    	if(createPreviewStream())
    	{
    		m_pCameraVTbl->ops->ch_release(m_pCameraVTbl->camera_handle, m_ChannelId);
    		m_ChannelId = 0;
			m_pCameraVTbl->ops->camera_close(m_pCameraVTbl->camera_handle);
			m_pCameraVTbl = 0;
			return system::errc::make_error_code(system::errc::resource_unavailable_try_again);
    	}

		#if 1
    	for(int p=MM_CAMERA_PARM_PICT_SIZE;p<MM_CAMERA_PARM_MAX;++p)
    	{
    		uint8_t supported;
    		m_pCameraVTbl->ops->is_parm_supported(m_pCameraVTbl->camera_handle,(mm_camera_parm_type_t)p,&supported,&supported);
    		std::cout << "param[" << p << "] " << (supported?"is    ":"is'nt ") << "supported!" << std::endl;
    	}
		#endif


    	return system::errc::make_error_code(system::errc::success);
    }

    system::error_code Camera::Impl::close()
    {
    	if(!isOpen())
    		return system::errc::make_error_code(system::errc::already_connected);

    	m_pCameraVTbl->ops->ch_release(m_pCameraVTbl->camera_handle, m_ChannelId);
    	m_ChannelId = 0;
    	m_pCameraVTbl->ops->camera_close(m_pCameraVTbl->camera_handle);
		m_pCameraVTbl = 0;
		return system::errc::make_error_code(system::errc::success);
    }

    std::vector<core::ePixelFormat> const& Camera::Impl::getSupportedPreviewFormats() const
    {
    	return m_SupportedPreviewFormats;
    }

    std::vector<core::ePixelFormat> const& Camera::Impl::getSupportedPictureFormats() const
	{
    	return m_SupportedPictureFormats;
	}

    std::vector<Vec2i> const& Camera::Impl::getSupportedPreviewSizes() const
    {
    	return m_SupportedPreviewSizes;
    }

    std::vector<Vec2i> const& Camera::Impl::getSupportedPictureSizes() const
	{
		return m_SupportedPictureSizes;
	}

    float Camera::Impl::getPreviewFPS() const
    {
    	return m_PreviewFPS;


    	/*if(m_pCameraVTbl->ops->get_parm(
    		m_pCameraVTbl->camera_handle,
    		MM_CAMERA_PARM_FPS,
    		&fr
    	))
    	{
    		system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
    	}
		*/
    }

    void Camera::Impl::setPreviewFPS(float fps)
    {
    	if(m_PreviewFPS == fps)
    		return;

    	m_PreviewFPS = fps;

    	if(isOpen())
    	{

    	}
    	/*
		if(m_pCameraVTbl->ops->get_parm(
			m_pCameraVTbl->camera_handle,
			MM_CAMERA_PARM_FPS,
			&fr
		))
		{
			system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}
		*/
    }

    core::ePixelFormat Camera::Impl::getPreviewFormat() const
    {
    	return m_PreviewFormat;
    }

    void Camera::Impl::setPreviewFormat(core::ePixelFormat fmt)
    {
    	if(m_PreviewFormat == fmt)
    		return;

    	m_PreviewFormat = fmt;
    }

    Vec2i const& Camera::Impl::getPreviewSize() const
    {
    	return m_PreviewSize;
    }

    void Camera::Impl::setPreviewSize(Vec2i const &sz)
    {
    	if(m_PreviewSize == sz)
    		return;

    	m_PreviewSize = sz;
    }


    core::ePixelFormat Camera::Impl::getPictureFormat() const
	{
		return m_PictureFormat;
	}

	void Camera::Impl::setPictureFormat(core::ePixelFormat fmt)
	{
		if(m_PictureFormat == fmt)
			return;

		m_PictureFormat = fmt;
	}

	Vec2i const& Camera::Impl::getPictureSize() const
	{
		return m_PictureSize;
	}

	void Camera::Impl::setPictureSize(Vec2i const &sz)
	{
		if(m_PictureSize == sz)
			return;

		m_PictureSize = sz;
	}

	system::error_code Camera::Impl::startPreview()
	{
		if(!isOpen())
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);

		if(isPreviewStarted())
			return system::errc::make_error_code(system::errc::already_connected);

		if(applyDimension())
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}

		mm_camera_op_mode_type_t op_mode=0;
		m_pCameraVTbl->ops->get_parm(m_pCameraVTbl->camera_handle, MM_CAMERA_PARM_OP_MODE, &op_mode);

		/*if(op_mode != MM_CAMERA_OP_MODE_VIDEO)
		{
			op_mode = MM_CAMERA_OP_MODE_VIDEO;
			if(m_pCameraVTbl->ops->set_parm(m_pCameraVTbl->camera_handle, MM_CAMERA_PARM_OP_MODE, &op_mode) < 0)
			{
				return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
			}
		}*/

		if(m_PreviewStream->create())
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}

		if(m_PreviewStream->start())
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}
		return system::errc::make_error_code(system::errc::success);
	}

	bool Camera::Impl::isPreviewStarted() const
	{
		return m_bPreviewStarted;
	}

	system::error_code Camera::Impl::stopPreview()
	{
		if(!isOpen())
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);

		if(!isPreviewStarted())
			return system::errc::make_error_code(system::errc::already_connected);

		m_PreviewStream->stop();
		m_PreviewStream->destroy();
		return system::errc::make_error_code(system::errc::success);
	}

    system::error_code Camera::Impl::init()
    {
    	uint32_t preview_sizes_count;

    	if(m_pCameraVTbl->ops->get_parm(m_pCameraVTbl->camera_handle, MM_CAMERA_PARM_PREVIEW_SIZES_CNT, &preview_sizes_count) < 0)
    	{
    		return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
    	}

    	std::vector<camera_size_type> default_preview_sizes;
    	default_preview_sizes.resize(preview_sizes_count);

    	default_sizes_tbl_t preview_sizes_tbl;
    	preview_sizes_tbl.tbl_size = preview_sizes_count;
    	preview_sizes_tbl.sizes_tbl = default_preview_sizes.data();

    	if(m_pCameraVTbl->ops->get_parm(m_pCameraVTbl->camera_handle, MM_CAMERA_PARM_DEF_PREVIEW_SIZES, &preview_sizes_tbl) < 0)
    	{
    		return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
    	}

    	mm_camera_dimension_t max_preview_size;
    	if(m_pCameraVTbl->ops->get_parm(m_pCameraVTbl->camera_handle, MM_CAMERA_PARM_MAX_PREVIEW_SIZE, &max_preview_size) < 0)
    	{
    		std::for_each(default_preview_sizes.begin(), default_preview_sizes.end(), [this](camera_size_type const &s)
			{
    			m_SupportedPreviewSizes.push_back(Vec2i(s.width, s.height));
			});
    	}
    	else
    	{
    		std::for_each(default_preview_sizes.begin(), default_preview_sizes.end(), [this, &max_preview_size](camera_size_type const &s)
			{
				if(s.width <= max_preview_size.width && s.height <= max_preview_size.height)
				{
					m_SupportedPreviewSizes.push_back(Vec2i(s.width,s.height));
				}
			});
    	}
    	m_PreviewSize = m_SupportedPreviewSizes[findBestSizeMatch(m_SupportedPreviewSizes, m_PreviewSize)];

    	mm_camera_dimension_t max_picture_size;
    	if(m_pCameraVTbl->ops->get_parm(m_pCameraVTbl->camera_handle, MM_CAMERA_PARM_MAX_PICTURE_SIZE, &max_picture_size) < 0)
    	{
    		m_MaxPictureSize = s_CameraDefaultPictureSizes[0];
    		for(uint32_t ips = 0; ips < s_CameraDefaultPictureSizesCount; ++ips)
    		{
    			m_SupportedPictureSizes.push_back(s_CameraDefaultPictureSizes[ips]);
    		}
    	}
    	else
    	{
    		m_MaxPictureSize = Vec2i(max_picture_size.width, max_picture_size.height);
    		for(uint32_t ips = 0; ips < sizeof(s_CameraDefaultPictureSizes)/sizeof(Vec2i); ++ips)
			{
    			if(	s_CameraDefaultPictureSizes[ips](0) > max_picture_size.width ||
    				s_CameraDefaultPictureSizes[ips](1) > max_picture_size.height)
    			{
    				continue;
    			}
    			m_SupportedPictureSizes.push_back(s_CameraDefaultPictureSizes[ips]);
			}
    	}
    	m_PictureSize = m_SupportedPictureSizes[findBestSizeMatch(m_SupportedPictureSizes, m_PictureSize)];

    	m_SupportedPreviewFormats.push_back(core::kPFMT_YUV_420_NV21);
    	m_PreviewFormat = core::kPFMT_YUV_420_NV21;

    	m_SupportedPictureFormats.push_back(core::kPFMT_YUV_420_NV21);
    	m_PictureFormat = core::kPFMT_YUV_420_NV21;

    	return system::errc::make_error_code(system::errc::success);
    }

    system::error_code Camera::Impl::setup()
    {

    	if(m_pCameraVTbl->ops->get_parm(m_pCameraVTbl->camera_handle, MM_CAMERA_PARM_DIMENSION, &m_CameraDimension.dim) < 0)
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}

    	m_CameraDimension.dim.display_width = m_PreviewSize(0);
    	m_CameraDimension.dim.display_height = m_PreviewSize(1);
    	m_CameraDimension.dim.prev_format = mapCoreToCameraPixelFormat(m_PreviewFormat);
    	m_CameraDimension.dim.prev_padding_format = CAMERA_PAD_TO_WORD;

    	m_CameraDimension.dim.enc_format = CAMERA_YUV_420_NV12;
    	m_CameraDimension.dim.orig_video_width = m_PictureSize(0);
    	m_CameraDimension.dim.orig_video_height = m_PictureSize(1);
    	m_CameraDimension.dim.video_width = m_PictureSize(0);
    	m_CameraDimension.dim.video_height = m_PictureSize(1);
    	m_CameraDimension.dim.video_chroma_width = m_PictureSize(0);
    	m_CameraDimension.dim.video_chroma_height  = m_PictureSize(1);

    	m_CameraDimension.dim.picture_width = m_PictureSize(0);
    	m_CameraDimension.dim.picture_height  = m_PictureSize(1);

    	m_CameraDimension.dim.ui_thumbnail_width = s_CameraDefaultStreamSize(0);
    	m_CameraDimension.dim.ui_thumbnail_height = s_CameraDefaultStreamSize(1);
    	m_CameraDimension.dim.main_img_format = CAMERA_YUV_420_NV21;
    	m_CameraDimension.dim.thumb_format = CAMERA_YUV_420_NV21;
    	m_CameraDimension.dim.rdi0_format = CAMERA_BAYER_SBGGR10;


    	if(m_pCameraVTbl->ops->set_parm(m_pCameraVTbl->camera_handle, MM_CAMERA_PARM_DIMENSION, &m_CameraDimension.dim) < 0)
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}

    	uint8_t frame_rate_mode_supported = 0;
    	if(m_pCameraVTbl->ops->is_parm_supported(m_pCameraVTbl->camera_handle,MM_CAMERA_PARM_FPS_MODE,&frame_rate_mode_supported,&frame_rate_mode_supported) == 0 && frame_rate_mode_supported)
    	{
			int32_t frame_rate_mode = 0;
			if(m_pCameraVTbl->ops->set_parm(m_pCameraVTbl->camera_handle, MM_CAMERA_PARM_FPS_MODE, &frame_rate_mode) < 0)
			{
				return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
			}
    	}

    	uint16_t frame_rate_fixed = m_PreviewFPS;
    	uint32_t frame_rate = frame_rate_fixed << 16 | frame_rate_fixed;
    	if(m_pCameraVTbl->ops->set_parm(m_pCameraVTbl->camera_handle, MM_CAMERA_PARM_FPS, &frame_rate) < 0)
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}


    	return system::errc::make_error_code(system::errc::success);
    }

    system::error_code Camera::Impl::createPreviewStream()
    {
    	m_PreviewStream = new PreviewStream(*this);
    	return system::errc::make_error_code(system::errc::success);
    }

    int32_t Camera::Impl::_getBuf(uint32_t camh, uint32_t cid, uint32_t sid, void *ud, mm_camera_frame_len_offset *foi, uint8_t nb, uint8_t *irf, mm_camera_buf_def_t *bufs)
    {
    	return reinterpret_cast<Camera::Impl*>(ud)->getBuf(camh, cid, sid, foi, nb, irf, bufs);
    }

    int32_t Camera::Impl::_putBuf(uint32_t camh, uint32_t cid, uint32_t sid, void *ud, uint8_t nb, mm_camera_buf_def_t *bufs)
    {
    	return reinterpret_cast<Camera::Impl*>(ud)->putBuf(camh, cid, sid, nb, bufs);
    }

    void Camera::Impl::_processHalEvent(uint32_t camh, mm_camera_event_t *e, void *ud)
    {
    	reinterpret_cast<Camera::Impl*>(ud)->processHalEvent(camh, e);
    }

    int32_t Camera::Impl::getBuf(uint32_t camh, uint32_t cid, uint32_t sid, mm_camera_frame_len_offset *foi, uint8_t nb, uint8_t *irf, mm_camera_buf_def_t *bufs)
    {
    	if(m_PreviewStream->getStreamId() == sid)
    		return m_PreviewStream->getBuf(foi, nb, irf, bufs);

    	return 0;
    }

    int32_t Camera::Impl::putBuf(uint32_t camh, uint32_t cid, uint32_t sid, uint8_t nb, mm_camera_buf_def_t *bufs)
    {
    	if(m_PreviewStream->getStreamId() == sid)
    		return m_PreviewStream->putBuf(nb, bufs);
    	return 0;
    }

    void Camera::Impl::processHalEvent(uint32_t camh, mm_camera_event_t *e)
	{
    	switch(e->event_type)
    	{
    	case MM_CAMERA_EVT_TYPE_CH:
    		processChannelEvent(&e->e.ch);
    		break;

    	case MM_CAMERA_EVT_TYPE_CTRL:
    		processCtrlEvent(&e->e.ctrl);
    		break;

    	case MM_CAMERA_EVT_TYPE_STATS:
    		processStatsEvent(&e->e.stats);
    		break;

    	case MM_CAMERA_EVT_TYPE_INFO:
    		processInfoEvent(&e->e.info);
    		break;

    	default:
    		break;
    	}
	}

    void Camera::Impl::processChannelEvent(mm_camera_ch_event_t *e)
    {
    	switch(e->ch)
    	{
    	case MM_CAMERA_PREVIEW:
    		processPreviewChannelEvent(e->evt);
    		break;

    	case MM_CAMERA_RDI:
			processRdiChannelEvent(e->evt);
			break;

		case MM_CAMERA_VIDEO:
			processRecordChannelEvent(e->evt);
			break;

		case MM_CAMERA_SNAPSHOT_MAIN:
		case MM_CAMERA_SNAPSHOT_THUMBNAIL:
			processSnapshotChannelEvent(e->evt);
			break;

		default:
			break;
		}
    }

    void Camera::Impl::processCtrlEvent(mm_camera_ctrl_event_t *e)
    {
    }

    void Camera::Impl::processStatsEvent(mm_camera_stats_event_t *e)
    {
    }

    void Camera::Impl::processInfoEvent(mm_camera_info_event_t *e)
    {
    }

    void Camera::Impl::processPreviewChannelEvent(mm_camera_ch_event_type_t che)
    {
    }

	void Camera::Impl::processRdiChannelEvent(mm_camera_ch_event_type_t che)
	{
	}

	void Camera::Impl::processRecordChannelEvent(mm_camera_ch_event_type_t che)
	{
	}

	void Camera::Impl::processSnapshotChannelEvent(mm_camera_ch_event_type_t che)
	{
	}

	system::error_code Camera::Impl::applyDimension()
	{
		if(m_pCameraVTbl->ops->get_parm(m_pCameraVTbl->camera_handle, MM_CAMERA_PARM_DIMENSION, &m_CameraDimension.dim) < 0)
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}

		//preview
		m_CameraDimension.dim.display_width = m_PreviewSize(0);
		m_CameraDimension.dim.display_height = m_PreviewSize(1);
		m_CameraDimension.dim.prev_format = mapCoreToCameraPixelFormat(m_PreviewFormat);
		m_CameraDimension.dim.prev_padding_format = CAMERA_PAD_TO_WORD;

		//video
		m_CameraDimension.dim.enc_format = CAMERA_YUV_420_NV12;
		m_CameraDimension.dim.orig_video_width = m_PictureSize(0);
		m_CameraDimension.dim.orig_video_height = m_PictureSize(1);
		m_CameraDimension.dim.video_width = m_PictureSize(0);
		m_CameraDimension.dim.video_height = m_PictureSize(1);
		m_CameraDimension.dim.video_chroma_width = m_PictureSize(0);
		m_CameraDimension.dim.video_chroma_height  = m_PictureSize(1);

		m_CameraDimension.dim.picture_width = m_PictureSize(0);
		m_CameraDimension.dim.picture_height  = m_PictureSize(1);

		m_CameraDimension.dim.ui_thumbnail_width = s_CameraDefaultStreamSize(0);
		m_CameraDimension.dim.ui_thumbnail_height = s_CameraDefaultStreamSize(1);
		m_CameraDimension.dim.main_img_format = CAMERA_YUV_420_NV21;
		m_CameraDimension.dim.thumb_format = CAMERA_YUV_420_NV21;
		m_CameraDimension.dim.rdi0_format = CAMERA_BAYER_SBGGR10;

		if(m_pCameraVTbl->ops->set_parm(m_pCameraVTbl->camera_handle, MM_CAMERA_PARM_DIMENSION, &m_CameraDimension.dim) < 0)
		{
			return system::errc::make_error_code(system::errc::inappropriate_io_control_operation);
		}

		m_PreviewStream->setFormat(m_CameraDimension.dim.prev_format);
		m_PreviewStream->setSize(m_PreviewSize);

		return system::errc::make_error_code(system::errc::success);
	}


} //namespace hal
} //namespace uquad
