#ifndef UQUAD_MEDIA_VIDEO_ENCODER_H
#define UQUAD_MEDIA_VIDEO_ENCODER_H

#include "Config.h"
#include "VideoCodec.h"

namespace uquad
{
namespace media
{
    class VideoEncoder
    	: public VideoCodec
    {
    public:
        
        class Registry
            : public VideoCodec::Registry
        {
        public:
            Registry(std::string const &n);
            ~Registry();
        };

    	VideoEncoder();
    	~VideoEncoder();
    };

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_VIDEO_ENCODER_H
