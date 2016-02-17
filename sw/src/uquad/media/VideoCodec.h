#ifndef UQUAD_MEDIA_VIDEO_CODEC_H
#define UQUAD_MEDIA_VIDEO_CODEC_H

#include "Config.h"
#include "Codec.h"

namespace uquad
{
namespace media
{
    class VideoCodec
    	: public Codec
    {
    public:
        
        class Registry
            : public Codec::Registry
        {
        public:
            Registry(std::string const &n, Component::Type const &type);
            ~Registry();
            
            SampleType getSampleType() const { return SampleType(getVideoCodecType()); }
            virtual eVideoCodecType getVideoCodecType() const = 0;
        };
        
    	VideoCodec();
    	~VideoCodec();
        
        SampleType getSampleType() const { return SampleType(getVideoCodecType()); }
        virtual eVideoCodecType getVideoCodecType() const = 0;
    
    protected:
    };

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_VIDEO_ENCODER_H
