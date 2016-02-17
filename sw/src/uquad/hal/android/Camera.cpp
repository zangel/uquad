#include "Camera.h"
#include "../DeviceManager.h"
#include "../Log.h"
#include "../../base/Error.h"
#include "../../utility/Constant.h"
#include "../../base/Memory.h"
#include "../../base/FileMappingMemory.h"
#include "../../base/MemoryBuffer.h"
#include "ASharedMemory.h"
#include "RawPixelSampleBuffer.h"
#include "MetaPixelSampleBuffer.h"


namespace uquad
{
namespace hal
{

	template<>
	system::error_code Device::registerDevices<Camera>(DeviceManager &dm)
	{
		hw_module_t *module = 0;
		int res = hw_get_module(CAMERA_HARDWARE_MODULE_ID, (const hw_module_t **)&module);

		if(res || !module)
			return base::makeErrorCode(base::kENoSuchDevice);

		camera_module_t *camera_module = reinterpret_cast<camera_module_t*>(module);
		int num_cameras = camera_module->get_number_of_cameras();

		for(int ic = 0; ic < num_cameras; ++ic)
		{
			camera_info info;
			res = camera_module->get_camera_info(ic, &info);
			if(res)
			{
				continue;
			}

			dm.registerDevice(
				intrusive_ptr<Camera>(new android::Camera(camera_module, ic))
			);
		}
		return base::makeErrorCode(base::kENoError);
	}

namespace android
{
    class Camera::CameraMemory
        : public base::RefCounted
    {
    public:
        CameraMemory(int fd, std::size_t buffer_size, std::size_t num_buffers)
            : m_Memory(
                new base::FileMappingMemory(
                    new base::FileHandle(fd),
                    base::FileHandle::kReadWrite,
                    buffer_size * num_buffers
                )
              )
            , m_Buffers()
        {
            ::memset(&m_CameraMemory, 0, sizeof(m_CameraMemory));
            if(m_Memory->isValid())
            {
                createBuffers(buffer_size, num_buffers);
            }
        }

        CameraMemory(std::size_t buffer_size, std::size_t num_buffers)
            : m_Memory(new ASharedMemory(buffer_size * num_buffers))
            , m_Buffers()
        {
            ::memset(&m_CameraMemory, 0, sizeof(m_CameraMemory));
            if(m_Memory->isValid())
            {
                createBuffers(buffer_size, num_buffers);
            }
        }

        ~CameraMemory()
        {
            m_Buffers.clear();
            m_Memory.reset();
        }

        inline std::size_t buffersCount() const { return m_Buffers.size(); }

        inline intrusive_ptr<base::Buffer> const& getBuffer(std::size_t index) const
        {
            return m_Buffers[index];
        }

        static CameraMemory* getCameraMemory(const camera_memory_t *cm)
        {
            return reinterpret_cast<CameraMemory*>(cm->handle);
        }

    private:
        void createBuffers(std::size_t buffer_size, std::size_t num_buffers)
        {
            m_Buffers.resize(num_buffers);
            for(std::size_t i = 0; i < m_Buffers.size();++i)
            {
                m_Buffers[i].reset(new base::MemoryBuffer(m_Memory, i * buffer_size, buffer_size));
            }

            m_CameraMemory.data = m_Memory->data();
            m_CameraMemory.size = m_Memory->size();
            m_CameraMemory.handle = this;
            m_CameraMemory.release = &CameraMemory::_releaseMemory;
        }

        static void _releaseMemory(camera_memory_t *data)
        {
            if (!data)
                return;
            CameraMemory *mem = reinterpret_cast<CameraMemory*>(data->handle);
            intrusive_ptr_release(mem);
        }

    private:
        intrusive_ptr<base::Memory> m_Memory;
        std::vector< intrusive_ptr<base::Buffer> > m_Buffers;

    public:
        camera_memory_t m_CameraMemory;
    };


	Camera::Camera(camera_module_t *m, int cidx)
		: hal::Camera()
		, m_CameraModule(m)
		, m_CameraIndex(cidx)
		, m_CameraDevice(0)
		, m_Name((format("Camera%1%")%cidx).str())
		, m_ConfigurationBegun(false)
		, m_CameraParameters()
	{
	}

	Camera::~Camera()
	{
	}

	std::string const& Camera::name() const
	{
		return m_Name;
	}

	bool Camera::isOpen() const
	{
		return m_CameraDevice != 0;
	}

	system::error_code Camera::open()
	{
		if(m_CameraDevice)
		{
		    UQUAD_HAL_LOG(Error) << "Camera::open: camera already opened!";
		    return base::makeErrorCode(base::kEAlreadyOpened);
		}

		std::string camId = lexical_cast<std::string>(m_CameraIndex);
		hw_device_t *device = 0;
		int openRes = m_CameraModule->common.methods->open((const hw_module_t*)m_CameraModule, camId.c_str(), &device);

		if(openRes || !device)
		{
		    UQUAD_HAL_LOG(Error) << "Camera::open: could not open camera module!";
		    return base::makeErrorCode(base::kENoSuchDevice);
		}

		camera_device_t *cam_device = reinterpret_cast<camera_device_t*>(device);;

		char *pParams = cam_device->ops->get_parameters(cam_device);

		if(!pParams)
		{
		    UQUAD_HAL_LOG(Error) << "Camera::open: could not get camera parameters!";
			cam_device->common.close(reinterpret_cast<hw_device_t*>(cam_device));
			return base::makeErrorCode(base::kEInvalidContent);
		}
		std::string strParams(pParams);
		cam_device->ops->put_parameters(cam_device, pParams);

		if(m_CameraParameters.parse(strParams) || !m_CameraParameters.isValid())
		{
		    UQUAD_HAL_LOG(Error) << "Camera::open: could not parse camera parameters!";
		    cam_device->common.close(reinterpret_cast<hw_device_t*>(cam_device));
			return base::makeErrorCode(base::kEInvalidContent);
		}

		m_CameraParameters.removeOutOfRangeFPS();
		strParams.clear();

		m_CameraParameters.focusMode = CameraParameters::kFocusModeInfinity;
		m_CameraParameters.noDisplayMode = true;
		//m_CameraParameters.previewFlip = CameraParameters::kFlipModeHorizontalAndVertical;
		//m_CameraParameters.videoFlip = CameraParameters::kFlipModeHorizontalAndVertical;

		float previewFPS = m_CameraParameters.previewFPS.get();
		m_CameraParameters.previewFPSRange = Vec2i(previewFPS*1000, previewFPS*1000);

		if(m_CameraParameters.generate(strParams))
		{
		    UQUAD_HAL_LOG(Error) << "Camera::open: could not generate camera parameters!";
			cam_device->common.close(reinterpret_cast<hw_device_t*>(cam_device));
			return base::makeErrorCode(base::kEInvalidContent);
		}

		if(cam_device->ops->set_parameters(cam_device, strParams.c_str()) < 0)
		{
		    UQUAD_HAL_LOG(Error) << "Camera::open: could not set camera parameters!";
			return base::makeErrorCode(base::kEInvalidArgument);
		}

		m_CameraDevice = cam_device;
		m_CameraDevice->ops->set_callbacks(m_CameraDevice,
			&Camera::_cameraNotifyCallback,
			&Camera::_cameraDataCallback,
			&Camera::_cameraDataTimestampCallback,
			&Camera::_cameraRequestMemory,
			this
		);

		if(m_CameraDevice->ops->store_meta_data_in_buffers)
		{
		    UQUAD_HAL_LOG(Debug) << "Camera::open: using meta data in buffers";
		    m_CameraDevice->ops->store_meta_data_in_buffers(m_CameraDevice, 1);
		}

		notifyOnDeviceOpened();
		m_ConfigurationBegun = false;

		return base::makeErrorCode(base::kENoError);
	}

	system::error_code Camera::close()
	{
		if(!m_CameraDevice)
		{
		    UQUAD_HAL_LOG(Error) << "Camera::close: camera not opened!";
		    return base::makeErrorCode(base::kENotOpened);
		}

		m_CameraDevice->common.close(reinterpret_cast<hw_device_t*>(m_CameraDevice));
		m_CameraDevice = 0;
		m_ConfigurationBegun = false;
		m_CameraParameters.reset();
		notifyOnDeviceClosed();
		return base::makeErrorCode(base::kENoError);
	}


	system::error_code Camera::beginConfiguration()
	{
		if(!isOpen())
		{
		    UQUAD_HAL_LOG(Error) << "Camera::beginConfiguration: camera not open!";
		    return base::makeErrorCode(base::kENotOpened);
		}

		if(m_ConfigurationBegun)
		{
		    UQUAD_HAL_LOG(Error) << "Camera::beginConfiguration: configuration already begun!";
			return base::makeErrorCode(base::kEAlreadyStarted);
		}

		m_ConfigurationBegun = true;
		return base::makeErrorCode(base::kENoError);
	}

	system::error_code Camera::endConfiguration()
	{
		if(!isOpen())
		{
		    UQUAD_HAL_LOG(Error) << "Camera::endConfiguration: camera not opened!";
			return base::makeErrorCode(base::kENotOpened);
		}

		if(!m_ConfigurationBegun)
		{
		    UQUAD_HAL_LOG(Error) << "Camera::endConfiguration: configuration not begun!";
			return base::makeErrorCode(base::kENotStarted);
		}

		m_ConfigurationBegun = false;

		std::string strParams;
		if(m_CameraParameters.generate(strParams))
		{
		    UQUAD_HAL_LOG(Error) << "Camera::endConfiguration: failed to generate camera parameters!";
			return base::makeErrorCode(base::kEInvalidArgument);
		}

		if(m_CameraDevice->ops->set_parameters(m_CameraDevice, strParams.c_str()) < 0)
		{
		    UQUAD_HAL_LOG(Error) << "Camera::endConfiguration: failed to apply camera parameters!";
			return base::makeErrorCode(base::kEInvalidArgument);
		}

		//m_CameraDevice->ops->auto_focus(m_CameraDevice);
        //m_CameraDevice->ops->cancel_auto_focus(m_CameraDevice);
		return base::makeErrorCode(base::kENoError);
	}

	unordered_set<base::ePixelFormat> const& Camera::getSupportedPreviewFormats() const
	{
		return m_CameraParameters
			.supportedPreviewFormats.get_value_or(
				utility::Constant< unordered_set<base::ePixelFormat> >::value()
			);
	}

	unordered_set<Vec2i> const& Camera::getSupportedPreviewSizes() const
	{
		return m_CameraParameters
			.supportedPreviewSizes.get_value_or(
				utility::Constant< unordered_set<Vec2i> >::value()
			);
	}

	unordered_set<float> const& Camera::getSupportedPreviewFPS() const
	{
		return m_CameraParameters
			.supportedPreviewFPS.get_value_or(
				utility::Constant< unordered_set<float> >::value()
			);
	}

	base::ePixelFormat Camera::getPreviewFormat() const
	{
		return m_CameraParameters.previewFormat.get_value_or(base::kPixelFormatInvalid);
	}

	system::error_code Camera::setPreviewFormat(base::ePixelFormat fmt)
	{
		if(!m_ConfigurationBegun)
			return base::makeErrorCode(base::kENotApplicable);

		if(isPreviewStarted())
			return base::makeErrorCode(base::kEAlreadyStarted);

		if(fmt == m_CameraParameters.previewFormat.get())
			return base::makeErrorCode(base::kENoError);

		if(!m_CameraParameters.supportedPreviewFormats.get().count(fmt))
			return base::makeErrorCode(base::kEInvalidArgument);

		m_CameraParameters.previewFormat = fmt;
		return base::makeErrorCode(base::kENoError);
	}

	Vec2i Camera::getPreviewSize() const
	{
		return m_CameraParameters.previewSize.get_value_or(Vec2i(0,0));
	}

	system::error_code Camera::setPreviewSize(Vec2i const &sz)
	{
		if(!m_ConfigurationBegun)
			return base::makeErrorCode(base::kENotApplicable);

		if(isPreviewStarted())
			return base::makeErrorCode(base::kEAlreadyStarted);

		if(sz == m_CameraParameters.previewSize.get())
			return base::makeErrorCode(base::kENoError);

		if(!m_CameraParameters.supportedPreviewSizes.get().count(sz))
			return base::makeErrorCode(base::kEInvalidArgument);

		m_CameraParameters.previewSize = sz;
		return base::makeErrorCode(base::kENoError);
	}

	float Camera::getPreviewFPS() const
	{
		return m_CameraParameters.previewFPS.get_value_or(0.0f);
	}

	system::error_code Camera::setPreviewFPS(float fps)
	{
		if(!m_ConfigurationBegun)
			return base::makeErrorCode(base::kENotApplicable);

		if(isPreviewStarted())
			return base::makeErrorCode(base::kEAlreadyStarted);

		if(fps == m_CameraParameters.previewFPS.get())
			return base::makeErrorCode(base::kENoError);

		if(!m_CameraParameters.supportedPreviewFPS.get().count(fps))
			return base::makeErrorCode(base::kEInvalidArgument);

        if( fps < 1.0e-3f*m_CameraParameters.supportedPreviewFPSRange.get()[0] ||
            fps > 1.0e-3f*m_CameraParameters.supportedPreviewFPSRange.get()[1])
        {
            UQUAD_HAL_LOG(Error) << "Camera::setPreviewFPS: " << fps << "," << m_CameraParameters.supportedPreviewFPSRange.get()[0] << "," << m_CameraParameters.supportedPreviewFPSRange.get()[1];
            return base::makeErrorCode(base::kEInvalidArgument);
        }

        m_CameraParameters.previewFPS = fps;
        m_CameraParameters.previewFPSRange = Vec2i(fps*1000, fps*1000);

		return base::makeErrorCode(base::kENoError);
	}

	unordered_set<base::ePixelFormat> const& Camera::getSupportedVideoFormats() const
	{
	    return utility::Constant< unordered_set<base::ePixelFormat> >::value();
	}

    unordered_set<Vec2i> const& Camera::getSupportedVideoSizes() const
    {
        return m_CameraParameters
            .supportedVideoSizes.get_value_or(
                utility::Constant< unordered_set<Vec2i> >::value()
            );
    }

    base::ePixelFormat Camera::getVideoFormat() const
    {
        return m_CameraParameters.videoFormat.get_value_or(base::kPixelFormatInvalid);
    }

    system::error_code Camera::setVideoFormat(base::ePixelFormat fmt)
    {
        return base::makeErrorCode(base::kENotApplicable);
    }

    Vec2i Camera::getVideoSize() const
    {
        return m_CameraParameters.videoSize.get_value_or(Vec2i(0,0));
    }

    system::error_code Camera::setVideoSize(Vec2i const &sz)
    {
        if(!m_ConfigurationBegun)
            return base::makeErrorCode(base::kENotApplicable);

        if(isRecordingStarted())
            return base::makeErrorCode(base::kEAlreadyStarted);

        if(sz == m_CameraParameters.videoSize.get())
            return base::makeErrorCode(base::kENoError);

        if(!m_CameraParameters.supportedVideoSizes.get().count(sz))
            return base::makeErrorCode(base::kEInvalidArgument);

        m_CameraParameters.videoSize = sz;
        return base::makeErrorCode(base::kENoError);
    }

	unordered_set<base::eImageFormat> const& Camera::getSupportedPictureFormats() const
	{
		return m_CameraParameters
			.supportedPictureFormats.get_value_or(
				utility::Constant< unordered_set<base::eImageFormat> >::value()
			);
	}

	unordered_set<Vec2i> const& Camera::getSupportedPictureSizes() const
	{
		return m_CameraParameters
			.supportedPictureSizes.get_value_or(
				utility::Constant< unordered_set<Vec2i> >::value()
			);
	}

	base::eImageFormat Camera::getPictureFormat() const
	{
		return m_CameraParameters.pictureFormat.get_value_or(base::kImageFormatInvalid);
	}

	system::error_code Camera::setPictureFormat(base::eImageFormat fmt)
	{
		if(!m_ConfigurationBegun)
			return base::makeErrorCode(base::kENotApplicable);

		if(fmt == m_CameraParameters.pictureFormat.get())
			return base::makeErrorCode(base::kENoError);

		if(!m_CameraParameters.supportedPictureFormats.get().count(fmt))
			return base::makeErrorCode(base::kEInvalidArgument);

		m_CameraParameters.pictureFormat = fmt;
		return makeErrorCode(base::kENoError);
	}

	Vec2i Camera::getPictureSize() const
	{
		return m_CameraParameters.pictureSize.get_value_or(Vec2i(0,0));
	}

	system::error_code Camera::setPictureSize(Vec2i const &sz)
	{
		if(!m_ConfigurationBegun)
			return base::makeErrorCode(base::kENotApplicable);

		if(sz == m_CameraParameters.pictureSize.get())
			return base::makeErrorCode(base::kENoError);

		if(!m_CameraParameters.supportedPictureSizes.get().count(sz))
			return base::makeErrorCode(base::kEInvalidArgument);

		m_CameraParameters.pictureSize = sz;
		return base::makeErrorCode(base::kENoError);
	}

	system::error_code Camera::startPreview()
	{
		if(!isOpen())
			return base::makeErrorCode(base::kENotOpened);

		if(isPreviewStarted())
			return base::makeErrorCode(base::kEAlreadyStarted);

		if(m_CameraDevice->ops->start_preview(m_CameraDevice) < 0)
			return base::makeErrorCode(base::kENotStarted);

        m_CameraDevice->ops->enable_msg_type(m_CameraDevice, CAMERA_MSG_PREVIEW_FRAME|CAMERA_MSG_FOCUS|CAMERA_MSG_FOCUS_MOVE);

        if(!beginConfiguration())
        {
            endConfiguration();
        }

        return base::makeErrorCode(base::kENoError);
	}

	bool Camera::isPreviewStarted() const
	{
		if(!isOpen())
			return false;
		return m_CameraDevice->ops->preview_enabled(m_CameraDevice)?true:false;
	}

	system::error_code Camera::stopPreview()
	{
		if(!isOpen())
			return base::makeErrorCode(base::kENotOpened);

		if(!isPreviewStarted())
			return base::makeErrorCode(base::kENotStarted);

        stopRecording();

        m_CameraDevice->ops->disable_msg_type(m_CameraDevice, CAMERA_MSG_PREVIEW_FRAME|CAMERA_MSG_FOCUS|CAMERA_MSG_FOCUS_MOVE);
        m_CameraDevice->ops->stop_preview(m_CameraDevice);

		return base::makeErrorCode(base::kENoError);
	}

	system::error_code Camera::startRecording()
	{
        if(!isOpen())
            return base::makeErrorCode(base::kENotOpened);

        if(isRecordingStarted())
            return base::makeErrorCode(base::kEAlreadyStarted);

        if(!isPreviewStarted())
            return base::makeErrorCode(base::kENotStarted);

        if(m_CameraDevice->ops->start_recording(m_CameraDevice) < 0)
            return base::makeErrorCode(base::kENotStarted);

        m_CameraDevice->ops->enable_msg_type(m_CameraDevice, CAMERA_MSG_VIDEO_FRAME);

        if(!beginConfiguration())
        {
            endConfiguration();
        }

        return base::makeErrorCode(base::kENoError);
	}

	bool Camera::isRecordingStarted() const
	{
        if(!isOpen())
            return false;
        return m_CameraDevice->ops->recording_enabled(m_CameraDevice)?true:false;
	}

    system::error_code Camera::stopRecording()
    {
        if(!isOpen())
            return base::makeErrorCode(base::kENotOpened);

        if(!isRecordingStarted())
            return base::makeErrorCode(base::kENotStarted);

        m_CameraDevice->ops->disable_msg_type(m_CameraDevice, CAMERA_MSG_VIDEO_FRAME);
        m_CameraDevice->ops->stop_recording(m_CameraDevice);

        return base::makeErrorCode(base::kENoError);
    }

	void Camera::cameraNotifyCallback(int32_t msg_type, int32_t ext1, int32_t ext2)
	{
	    switch(msg_type)
	    {
	    case CAMERA_MSG_FOCUS_MOVE:
	        {
                //m_CameraDevice->ops->cancel_auto_focus(m_CameraDevice);
	        }
	        break;

	    case CAMERA_MSG_FOCUS:
	        {
	            m_CameraDevice->ops->cancel_auto_focus(m_CameraDevice);
	        }
	        break;



	    default:
	        break;
	    }
	}

	void Camera::cameraDataCallback(int32_t msg_type, const camera_memory_t *data, unsigned int index, camera_frame_metadata_t *metadata)
	{
	    CameraMemory *pCamMem = CameraMemory::getCameraMemory(data);
        if(!pCamMem) return;

        intrusive_ptr<base::Buffer> buffer = pCamMem->getBuffer(index);
        if(!buffer || !buffer->isValid())
            return;

		switch(msg_type)
		{
		case CAMERA_MSG_PREVIEW_FRAME:
		    {
                notifyOnCameraPreviewFrame(intrusive_ptr<base::PixelSampleBuffer>(new RawPixelSampleBuffer(
                        buffer,
                        m_CameraParameters.previewSize.get(),
                        m_CameraParameters.previewFormat.get()
                    )
                ));
			}
			break;

		default:
			break;
		}
	}

	void Camera::cameraDataTimestampCallback(int64_t timestamp, int32_t msg_type, const camera_memory_t *data, unsigned int index)
	{
	    CameraMemory *pCamMem = CameraMemory::getCameraMemory(data);
        if(!pCamMem) return;
        intrusive_ptr<base::Buffer> buffer = pCamMem->getBuffer(index);
        if(!buffer || !buffer->isValid())
            return;

        if(msg_type == CAMERA_MSG_VIDEO_FRAME)
        {
            notifyOnCameraVideoFrame(intrusive_ptr<base::PixelSampleBuffer>(new MetaPixelSampleBuffer(
                    shared_ptr<::android::encoder_media_buffer_type>(
                        reinterpret_cast<::android::encoder_media_buffer_type*>(buffer->data()),
                        bind(m_CameraDevice->ops->release_recording_frame, m_CameraDevice, _1)
                    ),
                    m_CameraParameters.videoSize.get(),
                    m_CameraParameters.videoFormat.get()
                )
            ));
        }
	}

	camera_memory_t* Camera::cameraRequestMemory(int fd, size_t buf_size, unsigned int num_bufs)
	{
		if(fd < 0)
		{
		    CameraMemory *pCameraMemory = new CameraMemory(buf_size, num_bufs);
			intrusive_ptr_add_ref(pCameraMemory);
            return &pCameraMemory->m_CameraMemory;
		}
		else
		{
			CameraMemory *pCameraMemory = new CameraMemory(fd, buf_size, num_bufs);
			intrusive_ptr_add_ref(pCameraMemory);
			return &pCameraMemory->m_CameraMemory;
		}
	}

	void Camera::_cameraNotifyCallback(int32_t msg_type, int32_t ext1, int32_t ext2, void *user)
	{
		reinterpret_cast<Camera*>(user)->cameraNotifyCallback(msg_type, ext1, ext2);
	}

	void Camera::_cameraDataCallback(int32_t msg_type, const camera_memory_t *data, unsigned int index, camera_frame_metadata_t *metadata, void *user)
	{
		reinterpret_cast<Camera*>(user)->cameraDataCallback(msg_type, data, index, metadata);
	}

	void Camera::_cameraDataTimestampCallback(int64_t timestamp, int32_t msg_type, const camera_memory_t *data, unsigned int index, void *user)
	{
		reinterpret_cast<Camera*>(user)->cameraDataTimestampCallback(timestamp, msg_type, data, index);
	}

	camera_memory_t* Camera::_cameraRequestMemory(int fd, size_t buf_size, unsigned int num_bufs, void *user)
	{
		return reinterpret_cast<Camera*>(user)->cameraRequestMemory(fd, buf_size, num_bufs);
	}

} //namespace android
} //namespace hal
} //namespace uquad
