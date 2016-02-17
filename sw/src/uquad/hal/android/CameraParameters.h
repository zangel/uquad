#ifndef UQUAD_HAL_ANDROID_CAMERA_PARAMETERS_H
#define UQUAD_HAL_ANDROID_CAMERA_PARAMETERS_H

#include "../Config.h"
#include "../../base/PixelFormat.h"
#include "../../base/ImageFormat.h"


namespace uquad
{
namespace hal
{
namespace android
{
	class CameraParameters
	{
	public:
        
        typedef enum
        {
            kFlipModeNone = 0,
            kFlipModeHorizontal,
            kFlipModeVertical,
            kFlipModeHorizontalAndVertical
        } eFlipMode;
        
        typedef enum
        {
            kFocusModeAuto,
            kFocusModeInfinity,
            kFocusModeNormal,
            kFocusModeMacro,
            kFocusModeContinuousPicture,
            kFocusModeContinuousVideo
        } eFocusMode;
        
        
        
        
		CameraParameters();
		CameraParameters(CameraParameters const &that);
		~CameraParameters();


		CameraParameters& operator =(CameraParameters const &rhs);

		bool isValid() const;
        void reset();
        
        CameraParameters& apply(CameraParameters const &rhs);

        void removeOutOfRangeFPS();

		system::error_code parse(std::string const &s);
		system::error_code generate(std::string &s);

		optional< unordered_set<base::ePixelFormat> > supportedPreviewFormats;
        optional<base::ePixelFormat> previewFormat;
		
		optional< unordered_set<Vec2i> > supportedPreviewSizes;
        optional<Vec2i> previewSize;
		
		optional< unordered_set<float> > supportedPreviewFPS;
        optional<float> previewFPS;
        
        optional<Vec2i> supportedPreviewFPSRange;
        optional<Vec2i> previewFPSRange;
        
        optional<bool> noDisplayMode;
        optional<eFlipMode> previewFlip;
        
        optional<base::ePixelFormat> videoFormat;
        
        optional< unordered_set<Vec2i> > supportedVideoSizes;
        optional<Vec2i> videoSize;
        
        optional<eFlipMode> videoFlip;

		optional< unordered_set<base::eImageFormat> > supportedPictureFormats;
        optional<base::eImageFormat> pictureFormat;
		
		optional< unordered_set<Vec2i> > supportedPictureSizes;
        optional<Vec2i> pictureSize;
        
        optional<eFocusMode> focusMode;
        
        std::map<std::string, std::string> unused;
	};

} //namespace android
} //namespace hal
} //namespace uquad


#endif //UQUAD_HAL_ANDROID_CAMERA_PARAMETERS_H
