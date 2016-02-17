#ifndef UQUAD_HAL_ANDROID_CAMERA_H
#define UQUAD_HAL_ANDROID_CAMERA_H

#include "../Config.h"
#include "../Camera.h"
#include <hardware/hardware.h>
#include <hardware/camera.h>
#include "CameraParameters.h"
#include "../../base/Buffer.h"


namespace uquad
{
namespace hal
{
namespace android
{
    class RawPixelSampleBuffer;
    class MetaPixelSampleBuffer;

    class Camera
    	: public hal::Camera
    {
    public:
        class CameraMemory;

        Camera(camera_module_t *m, int cidx);
        ~Camera();

        std::string const& name() const;

        bool isOpen() const;
        system::error_code open();
        system::error_code close();

        system::error_code beginConfiguration();
        system::error_code endConfiguration();

        unordered_set<base::ePixelFormat> const& getSupportedPreviewFormats() const;
        unordered_set<Vec2i> const& getSupportedPreviewSizes() const;
        unordered_set<float> const& getSupportedPreviewFPS() const;
        base::ePixelFormat getPreviewFormat() const;
        system::error_code setPreviewFormat(base::ePixelFormat fmt);
        Vec2i getPreviewSize() const;
        system::error_code setPreviewSize(Vec2i const &sz);
        float getPreviewFPS() const;
        system::error_code setPreviewFPS(float fps);

        unordered_set<base::ePixelFormat> const& getSupportedVideoFormats() const;
        unordered_set<Vec2i> const& getSupportedVideoSizes() const;
        base::ePixelFormat getVideoFormat() const;
        system::error_code setVideoFormat(base::ePixelFormat fmt);
        Vec2i getVideoSize() const;
        system::error_code setVideoSize(Vec2i const &sz);

        unordered_set<base::eImageFormat> const& getSupportedPictureFormats() const;
        unordered_set<Vec2i> const& getSupportedPictureSizes() const;
		base::eImageFormat getPictureFormat() const;
		system::error_code setPictureFormat(base::eImageFormat fmt);
		Vec2i getPictureSize() const;
		system::error_code setPictureSize(Vec2i const &sz);

		system::error_code startPreview();
		bool isPreviewStarted() const;
		system::error_code stopPreview();

		system::error_code startRecording();
        bool isRecordingStarted() const;
        system::error_code stopRecording();


    private:
		void cameraNotifyCallback(int32_t msg_type, int32_t ext1, int32_t ext2);
		void cameraDataCallback(int32_t msg_type, const camera_memory_t *data, unsigned int index, camera_frame_metadata_t *metadata);
		void cameraDataTimestampCallback(int64_t timestamp, int32_t msg_type, const camera_memory_t *data, unsigned int index);
		camera_memory_t* cameraRequestMemory(int fd, size_t buf_size, unsigned int num_bufs);

		static void _cameraNotifyCallback(int32_t msg_type, int32_t ext1, int32_t ext2, void *user);
		static void _cameraDataCallback(int32_t msg_type, const camera_memory_t *data, unsigned int index, camera_frame_metadata_t *metadata, void *user);
		static void _cameraDataTimestampCallback(int64_t timestamp, int32_t msg_type, const camera_memory_t *data, unsigned int index, void *user);
		static camera_memory_t* _cameraRequestMemory(int fd, size_t buf_size, unsigned int num_bufs, void *user);

    private:
		camera_module_t *m_CameraModule;
		int m_CameraIndex;
		camera_device_t *m_CameraDevice;

		std::string m_Name;
		bool m_ConfigurationBegun;
		CameraParameters m_CameraParameters;
    };

} //namespace android
} //namespace hal
} //namespace uquad


#endif //UQUAD_HAL_ANDROID_CAMERA_H
