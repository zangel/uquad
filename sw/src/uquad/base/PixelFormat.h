#ifndef UQUAD_BASE_PIXEL_FORMAT_H
#define UQUAD_BASE_PIXEL_FORMAT_H

#include "Config.h"

namespace uquad
{
namespace base
{
	typedef enum
	{
		kPixelFormatInvalid=0,
		kPixelFormatRGB8,
        kPixelFormatBGRA8,
		kPixelFormatYUV420sp,
		//unsupported formats:
		kPixelFormatYUV420spAdreno,
        kPixelFormatYUV420p,
        kPixelFormatYUV420NV12,
        kPixelFormatMax
	} ePixelFormat;

} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_PIXEL_FORMAT_H
