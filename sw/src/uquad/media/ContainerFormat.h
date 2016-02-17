#ifndef UQUAD_MEDIA_CONTAINER_FORMAT_H
#define UQUAD_MEDIA_CONTAINER_FORMAT_H

#include "Config.h"

namespace uquad
{
namespace media
{
    typedef enum
    {
        kContainerFormatInvalid,
        kContainerFormatRAW,
        kContainerFormatMOV,
        kContainerFormatMPEG4,
        kContainerFormatMPEGTS,
        kContainerFormatMax
    } eContainerFormat;

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_CONTAINER_FORMAT_H
