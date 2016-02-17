#ifndef UQUAD_MEDIA_SAMPLE_TYPE_H
#define UQUAD_MEDIA_SAMPLE_TYPE_H

#include "Config.h"
#include "../base/PixelFormat.h"
#include "CodecType.h"

namespace uquad
{
namespace media
{
    typedef enum
    {
        kSampleTypeInvalid,
        kSampleTypeAudioRaw,
        kSampleTypeVideoRaw,
        kSampleTypeAudioEncoded,
        kSampleTypeVideoEncoded,
        kSampleTypeMax
    } eSampleType;
    
   
    struct SampleType
    {
        SampleType()
        {
            type = kSampleTypeInvalid;
        }
        
        SampleType(base::ePixelFormat pfmt)
        {
            type = kSampleTypeVideoRaw;
            kind.videoRaw = pfmt;
        }
        
        SampleType(eVideoCodecType vct)
        {
            type = kSampleTypeVideoEncoded;
            kind.videoEncoded = vct;
        }
        
        inline bool operator==(SampleType const &rhs) const { return ::memcmp(this, &rhs, sizeof(SampleType)) == 0; }
        inline bool operator!=(SampleType const &rhs) const { return ::memcmp(this, &rhs, sizeof(SampleType)) != 0; }
        
        
        eSampleType type;
        union
        {
            base::ePixelFormat videoRaw;
            eVideoCodecType  videoEncoded;
        } kind;
    };
    

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_SAMPLE_TYPE_H
