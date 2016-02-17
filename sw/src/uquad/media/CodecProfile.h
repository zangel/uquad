#ifndef UQUAD_MEDIA_CODEC_PROFILE_H
#define UQUAD_MEDIA_CODEC_PROFILE_H

#include "Config.h"

namespace uquad
{
namespace media
{
    typedef enum
    {
        kCodecProfileInvalid,
        kCodecProfileLowLatency,
        kCodecProfileHighQuality,
        kCodecProfileMax
    } eCodecProfile;


} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_CODEC_PROFILE_H
