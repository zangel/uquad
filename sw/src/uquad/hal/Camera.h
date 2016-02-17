#ifndef UQUAD_HAL_CAMERA_H
#define UQUAD_HAL_CAMERA_H

#include "Config.h"
#include "Device.h"
#include "../base/PixelFormat.h"
#include "../base/ImageFormat.h"
#include "../base/PixelSampleBuffer.h"


namespace uquad
{
namespace hal
{
	class CameraDelegate
	{
	public:
		CameraDelegate() = default;
		virtual ~CameraDelegate() = default;
        
        virtual void onCameraParametersChanged() {}
        virtual void onCameraPreviewStarted() {}
        virtual void onCameraPreviewStopped() {}
        virtual void onCameraPreviewFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer) {}
        virtual void onCameraVideoFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer) {}
	};


    class Camera
    	: public Device
    {
    public:
    	typedef base::Delegate<CameraDelegate> camera_delegate_t;

    	Camera();
        ~Camera();

        Device::Type type() const;

        virtual system::error_code beginConfiguration() = 0;
        virtual system::error_code endConfiguration() = 0;
        virtual unordered_set<base::ePixelFormat> const& getSupportedPreviewFormats() const = 0;
        virtual unordered_set<Vec2i> const& getSupportedPreviewSizes() const = 0;
        virtual unordered_set<float> const& getSupportedPreviewFPS() const = 0;
        virtual base::ePixelFormat getPreviewFormat() const = 0;
        virtual system::error_code setPreviewFormat(base::ePixelFormat fmt) = 0;
        virtual Vec2i getPreviewSize() const = 0;
        virtual system::error_code setPreviewSize(Vec2i const &sz) = 0;
        virtual float getPreviewFPS() const = 0;
        virtual system::error_code setPreviewFPS(float fps) = 0;
        
        virtual unordered_set<base::ePixelFormat> const& getSupportedVideoFormats() const = 0;
        virtual unordered_set<Vec2i> const& getSupportedVideoSizes() const = 0;
        virtual base::ePixelFormat getVideoFormat() const = 0;
        virtual system::error_code setVideoFormat(base::ePixelFormat fmt) = 0;
        virtual Vec2i getVideoSize() const = 0;
        virtual system::error_code setVideoSize(Vec2i const &sz) = 0;
        

        virtual unordered_set<base::eImageFormat> const& getSupportedPictureFormats() const = 0;
        virtual unordered_set<Vec2i> const& getSupportedPictureSizes() const = 0;
        virtual base::eImageFormat getPictureFormat() const = 0;
        virtual system::error_code setPictureFormat(base::eImageFormat fmt) = 0;
        virtual Vec2i getPictureSize() const = 0;
        virtual system::error_code setPictureSize(Vec2i const &sz) = 0;

        virtual system::error_code startPreview() = 0;
        virtual bool isPreviewStarted() const = 0;
        virtual system::error_code stopPreview() = 0;
        
        virtual system::error_code startRecording() = 0;
        virtual bool isRecordingStarted() const = 0;
        virtual system::error_code stopRecording() = 0;

        inline camera_delegate_t const& cameraDelegate() { return m_CameraDelegate; }
        
    protected:
        
        inline void notifyOnCameraParametersChanged()
        {
            m_CameraDelegate.call(&CameraDelegate::onCameraParametersChanged);
        }
    
        inline void notifyOnCameraPreviewStarted()
        {
            m_CameraDelegate.call(&CameraDelegate::onCameraPreviewStarted);
        }
        
        inline void notifyOnCameraPreviewStopped()
        {
            m_CameraDelegate.call(&CameraDelegate::onCameraPreviewStopped);
        }
        
        inline void notifyOnCameraPreviewFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer)
        {
            m_CameraDelegate.call(&CameraDelegate::onCameraPreviewFrame, buffer);
        }
        
        inline void notifyOnCameraVideoFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer)
        {
            m_CameraDelegate.call(&CameraDelegate::onCameraVideoFrame, buffer);
        }

    private:
        camera_delegate_t m_CameraDelegate;
    };

} //namespace hal
} //namespace uquad

#endif //UQUAD_HAL_CAMERA_H
