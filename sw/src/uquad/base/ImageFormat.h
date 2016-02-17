#ifndef UQUAD_BASE_IMAGE_FORMAT_H
#define UQUAD_BASE_IMAGE_FORMAT_H

#include "Config.h"

namespace uquad
{
namespace base
{
    typedef enum
	{
		kImageFormatInvalid = 0,
		kImageFormatRAW,
		kImageFormatJPEG,
        kImageFormatMax
	} eImageFormat;
    
} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_IMAGE_FORMAT_H
