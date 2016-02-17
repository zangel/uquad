#ifndef UQUAD_MEDIA_CODEC_H
#define UQUAD_MEDIA_CODEC_H

#include "Config.h"
#include "Source.h"
#include "Sink.h"
#include "SampleType.h"
#include "CodecProfile.h"
#include "CodecType.h"


namespace uquad
{
namespace media
{
    class Codec
        : public Source
        , public Sink
    {
    public:
        Codec();
        ~Codec();
        
        class Registry
            : public Component::Registry
        {
        public:
            Registry(std::string const &n, Component::Type const &type);
            ~Registry();

            virtual SampleType getSampleType() const = 0;
        };

        virtual SampleType getSampleType() const = 0;
        virtual eCodecProfile getProfile() const = 0;
        virtual bool setProfile(eCodecProfile profile) = 0;
        virtual int getBitRate() const = 0;
        virtual bool setBitRate(int br) = 0;
    };

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_CODEC_H
