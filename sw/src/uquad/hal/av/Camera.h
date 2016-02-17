#ifndef UQUAD_HAL_AV_CAMERA_H
#define UQUAD_HAL_AV_CAMERA_H

#include "../Config.h"
#include "../Camera.h"
#include "../../base/Buffer.h"
#include <AVFoundation/AVFoundation.h>

@class uquad_hal_av_Camera;

namespace uquad
{
namespace hal
{
namespace av
{
    class Camera
    	: public hal::Camera
    {
    public:
    	Camera(AVCaptureDevice *dev);
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
        
        static system::error_code registerCameras(DeviceManager &dm);
        
    public:
        using hal::Camera::notifyOnCameraPreviewFrame;
        using hal::Camera::notifyOnCameraVideoFrame;
        
    private:
        void resetParameters();
        
    private:
		uquad_hal_av_Camera *m_Camera;
        std::string m_Name;
        
        bool m_bConfigurationBegun;
        unordered_set<base::ePixelFormat> m_SupportedPreviewFormats;
        unordered_set<Vec2i> m_SupportedPreviewSizes;
        unordered_set<float> m_SupportedPreviewFPS;
        base::ePixelFormat m_PreviewFormat;
        Vec2i m_PreviewSize;
        float m_PreviewFPS;
        
        unordered_set<base::ePixelFormat> m_SupportedVideoFormats;
        unordered_set<Vec2i> m_SupportedVideoSizes;
        
        unordered_set<base::eImageFormat> m_SupportedPictureFormats;
        unordered_set<Vec2i> m_SupportedPictureSizes;
        base::eImageFormat m_PictureFormat;
        Vec2i m_PictureSize;
    };

} //namespace av
} //namespace hal
} //namespace uquad


#endif //UQUAD_HAL_AV_CAMERA_H