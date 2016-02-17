#ifndef UQUAD_MEDIA_CODEC_TYPE_H
#define UQUAD_MEDIA_CODEC_TYPE_H

#include "Config.h"

namespace uquad
{
namespace media
{
    typedef enum
    {
        kVideoCodecTypeInvalid = 0,
        kVideoCodecTypeH263,
        kVideoCodecTypeH264,
        kVideoCodecTypeMPEG4,
        kVideoCodecTypeMax
    } eVideoCodecType;

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_CODEC_TYPE_H
