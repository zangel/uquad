#ifndef UQUAD_MEDIA_OMX_OMXCORE_H
#define UQUAD_MEDIA_OMX_OMXCORE_H

#include "../Config.h"
#include "../SampleType.h"
#include "../CodecProfile.h"
#include "../ContainerFormat.h"
#include "../ComponentFactory.h"
#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Video.h>
#include <OMX_QCOMExtns.h>
#include <media/hardware/HardwareAPI.h>


namespace uquad
{
namespace media
{
namespace omx
{
    struct ProfileLevel
    {
        std::size_t max_mb_per_frame;
        std::size_t max_mb_per_second;
        std::size_t max_bitrate;
        int level;
        int profile;
    };

    class OMXCore
    {
    public:
    protected:
        OMXCore();

    public:

        static char const* EXT_OMX_google_android_index_storeMetaDataInBuffers;
        static char const* EXT_OMX_google_android_index_prependSPSPPSToIDRFrames;

        inline bool isInitialized() const { return m_bInitialized; }

        std::size_t numComponents() const;
        system::error_code getComponentName(std::size_t index, std::string &name) const;
        system::error_code getComponentRoles(std::size_t index, std::vector<std::string> &roles) const;

        system::error_code createComponentHandle(OMX_HANDLETYPE &handle, std::size_t index, OMX_PTR data, OMX_CALLBACKTYPE *callbacks) const;
        system::error_code destroyComponentHandle(OMX_HANDLETYPE handle) const;

        system::error_code registerVideoEncoders(ComponentFactory &f);
        system::error_code registerVideoDecoders(ComponentFactory &f);

        static eVideoCodecType videoCodecTypeFromOMXRole(std::string const &name);
        static OMX_VIDEO_CODINGTYPE omxCodingTypeFromVideoCodecType(eVideoCodecType vct);

        static char const* h264ProfileName(OMX_VIDEO_AVCPROFILETYPE profile);
        static char const* h264LevelName(OMX_VIDEO_AVCLEVELTYPE level);

        static ProfileLevel const* chooseOMXProfileLevel(eVideoCodecType vct, eCodecProfile cp, Vec2i const &fs, float fps, std::size_t br);



    public:
       ~OMXCore();

    private:
       bool m_bInitialized;
    };

} //namespace omx
} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_OMX_OMXCORE_H
