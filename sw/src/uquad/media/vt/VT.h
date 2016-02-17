#ifndef UQUAD_MEDIA_VT_VT_H
#define UQUAD_MEDIA_VT_VT_H

#include "../Config.h"
#include "../SampleType.h"
#include "../ContainerFormat.h"
#include "../ComponentFactory.h"
#include "../CodecProfile.h"
#include <VideoToolbox/VideoToolbox.h>

namespace uquad
{
namespace media
{
namespace vt
{
    struct ProfileLevel
    {
        std::size_t max_mb_per_frame;
        std::size_t max_mb_per_second;
        std::size_t max_bitrate;
        CFStringRef profileLevel;
    };
    
	class VT
	{
	public:
	protected:
		VT();

	public:
		inline bool isInitialized() const { return m_bInitialized; }
        
        static std::string videoCodecNameFromEncoderSpec(CFDictionaryRef es);
        static eVideoCodecType videoCodecTypeFromEncoderSpec(CFDictionaryRef es);
        static CMVideoCodecType cmVideoCodecTypeFromVideoCodecType(eVideoCodecType vct);
        static ProfileLevel const* chooseVTProfileLevel(eVideoCodecType vct, eCodecProfile cp, Vec2i const &fs, float fps, std::size_t br);
        
        system::error_code registerVideoEncoders(ComponentFactory &f);
        system::error_code registerVideoDecoders(ComponentFactory &f);
    
    public:
       ~VT();

	private:
       bool m_bInitialized;
	};

} //namespace vt
} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_VT_VT_H
