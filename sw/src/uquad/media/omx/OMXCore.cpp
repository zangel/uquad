#include "OMXCore.h"
#include "../../base/Error.h"

#include "VideoEncoder.h"
#include "VideoDecoder.h"


#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <OMX_Core.h>
#include <qc_omx_core.h>
#include <omx_core_cmp.h>
#include <dlfcn.h>


extern omx_core_cb_type core[];
extern const unsigned int SIZE_OF_CORE;

namespace uquad
{
namespace media
{
namespace omx
{
    namespace
    {
        static ::create_qc_omx_component omx_core_load_cmp_library(char *libname, void **handle_ptr)
        {
            create_qc_omx_component fn_ptr = NULL;
            if(handle_ptr)
            {
                *handle_ptr = ::dlopen(libname, RTLD_NOW);
                if(*handle_ptr)
                {
                    fn_ptr = reinterpret_cast<::create_qc_omx_component>(::dlsym(*handle_ptr, "get_omx_component_factory_fn"));

                    if(fn_ptr == NULL)
                    {
                        *handle_ptr = NULL;
                    }
                }
                else
                {
                }
            }
            return fn_ptr;
        }

        static int get_comp_handle_index(char *cmp_name)
        {
            unsigned i=0,j=0;
            int rc = -1;
            for(i=0; i< SIZE_OF_CORE; i++)
            {
                if(!strcmp(cmp_name, ::core[i].name))
                {
                    for(j=0; j< OMX_COMP_MAX_INST; j++)
                    {
                        if(NULL == ::core[i].inst[j])
                        {
                            rc = j;
                            return rc;
                        }
                    }
                    break;
                }
            }
            return rc;
        }

        static int is_cmp_handle_exists(OMX_HANDLETYPE inst)
        {
            unsigned i=0,j=0;
            int rc = -1;

            if(NULL == inst)
                return rc;

            for(i=0; i< SIZE_OF_CORE; i++)
            {
                for(j=0; j< OMX_COMP_MAX_INST; j++)
                {
                    if(inst == ::core[i].inst[j])
                    {
                        rc = i;
                        return rc;
                    }
                }
            }
            return rc;
        }

        static int check_lib_unload(int index)
        {
            unsigned i=0;
            int rc = 1;

            for(i=0; i< OMX_COMP_MAX_INST; i++)
            {
                if(::core[index].inst[i])
                {
                    rc = 0;
                    break;
                }
            }
            return rc;
        }

        static void clear_cmp_handle(OMX_HANDLETYPE inst)
        {
            unsigned i = 0,j=0;

            if(NULL == inst)
                return;

            for(i=0; i< SIZE_OF_CORE; i++)
            {
                for(j=0; j< OMX_COMP_MAX_INST; j++)
                {
                    if(inst == ::core[i].inst[j])
                    {
                        ::core[i].inst[j] = NULL;
                        return;
                    }
                }
            }
            return;
        }

        static ProfileLevel g_ProfileLevelsTableMPEG4LowLatency[] =
        {
            {99,    1485,   64000,      OMX_VIDEO_MPEG4Level0,  OMX_VIDEO_MPEG4ProfileSimple},
            {99,    1485,   64000,      OMX_VIDEO_MPEG4Level1,  OMX_VIDEO_MPEG4ProfileSimple},
            {396,   5940,   128000,     OMX_VIDEO_MPEG4Level2,  OMX_VIDEO_MPEG4ProfileSimple},
            {396,   11880,  384000,     OMX_VIDEO_MPEG4Level3,  OMX_VIDEO_MPEG4ProfileSimple},
            {1200,  36000,  4000000,    OMX_VIDEO_MPEG4Level4a, OMX_VIDEO_MPEG4ProfileSimple},
            {1620,  40500,  8000000,    OMX_VIDEO_MPEG4Level5,  OMX_VIDEO_MPEG4ProfileSimple},
            {3600,  108000, 12000000,   OMX_VIDEO_MPEG4Level5,  OMX_VIDEO_MPEG4ProfileSimple}
        };
        static std::size_t const g_ProfileLevelsTableSizeMPEG4LowLatency = sizeof(g_ProfileLevelsTableMPEG4LowLatency)/sizeof(ProfileLevel);

        static ProfileLevel g_ProfileLevelsTableMPEG4HighQuality[] =
        {
            {99,    1485,   128000,     OMX_VIDEO_MPEG4Level0,  OMX_VIDEO_MPEG4ProfileAdvancedSimple},
            {99,    1485,   128000,     OMX_VIDEO_MPEG4Level1,  OMX_VIDEO_MPEG4ProfileAdvancedSimple},
            {396,   5940,   384000,     OMX_VIDEO_MPEG4Level2,  OMX_VIDEO_MPEG4ProfileAdvancedSimple},
            {396,   11880,  768000,     OMX_VIDEO_MPEG4Level3,  OMX_VIDEO_MPEG4ProfileAdvancedSimple},
            {792,   23760,  3000000,    OMX_VIDEO_MPEG4Level4,  OMX_VIDEO_MPEG4ProfileAdvancedSimple},
            {1620,  48600,  8000000,    OMX_VIDEO_MPEG4Level5,  OMX_VIDEO_MPEG4ProfileAdvancedSimple}
        };
        static std::size_t const g_ProfileLevelsTableSizeMPEG4HighQuality = sizeof(g_ProfileLevelsTableMPEG4HighQuality)/sizeof(ProfileLevel);

        static ProfileLevel g_ProfileLevelsTableH263[] =
        {
            {99,    1485,   64000,      OMX_VIDEO_H263Level10,  OMX_VIDEO_H263ProfileBaseline},
            {396,   5940,   128000,     OMX_VIDEO_H263Level20,  OMX_VIDEO_H263ProfileBaseline},
            {396,   11880,  384000,     OMX_VIDEO_H263Level30,  OMX_VIDEO_H263ProfileBaseline},
            {396,   11880,  2048000,    OMX_VIDEO_H263Level40,  OMX_VIDEO_H263ProfileBaseline},
            {99,    1485,   128000,     OMX_VIDEO_H263Level45,  OMX_VIDEO_H263ProfileBaseline},
            {396,   19800,  4096000,    OMX_VIDEO_H263Level50,  OMX_VIDEO_H263ProfileBaseline},
            {810,   40500,  8192000,    OMX_VIDEO_H263Level60,  OMX_VIDEO_H263ProfileBaseline},
            {1620,  81000,  16384000,   OMX_VIDEO_H263Level70,  OMX_VIDEO_H263ProfileBaseline}
        };
        static std::size_t const g_ProfileLevelsTableSizeH263 = sizeof(g_ProfileLevelsTableH263)/sizeof(ProfileLevel);

        static ProfileLevel g_ProfileLevelsTableH264LowLatency[] =
        {
            {99,    1485,   64000,      OMX_VIDEO_AVCLevel1,    OMX_VIDEO_AVCProfileBaseline},
            {99,    1485,   128000,     OMX_VIDEO_AVCLevel1b,   OMX_VIDEO_AVCProfileBaseline},
            {396,   3000,   192000,     OMX_VIDEO_AVCLevel11,   OMX_VIDEO_AVCProfileBaseline},
            {396,   6000,   384000,     OMX_VIDEO_AVCLevel12,   OMX_VIDEO_AVCProfileBaseline},
            {396,   11880,  768000,     OMX_VIDEO_AVCLevel13,   OMX_VIDEO_AVCProfileBaseline},
            {396,   11880,  2000000,    OMX_VIDEO_AVCLevel2,    OMX_VIDEO_AVCProfileBaseline},
            {792,   19800,  4000000,    OMX_VIDEO_AVCLevel21,   OMX_VIDEO_AVCProfileBaseline},
            {1620,  20250,  4000000,    OMX_VIDEO_AVCLevel22,   OMX_VIDEO_AVCProfileBaseline},
            {1620,  40500,  10000000,   OMX_VIDEO_AVCLevel3,    OMX_VIDEO_AVCProfileBaseline},
            {3600,  108000, 14000000,   OMX_VIDEO_AVCLevel31,   OMX_VIDEO_AVCProfileBaseline},
            {5120,  216000, 20000000,   OMX_VIDEO_AVCLevel32,   OMX_VIDEO_AVCProfileBaseline},
            {8192,  245760, 20000000,   OMX_VIDEO_AVCLevel4,    OMX_VIDEO_AVCProfileBaseline}
        };
        static std::size_t const g_ProfileLevelsTableSizeH264LowLatency = sizeof(g_ProfileLevelsTableH264LowLatency)/sizeof(ProfileLevel);

        static ProfileLevel g_ProfileLevelsTableH264HighQuality[] =
        {
            {99,    1485,   64000,      OMX_VIDEO_AVCLevel1,    OMX_VIDEO_AVCProfileHigh},
            {99,    1485,   160000,     OMX_VIDEO_AVCLevel1b,   OMX_VIDEO_AVCProfileHigh},
            {396,   3000,   240000,     OMX_VIDEO_AVCLevel11,   OMX_VIDEO_AVCProfileHigh},
            {396,   6000,   480000,     OMX_VIDEO_AVCLevel12,   OMX_VIDEO_AVCProfileHigh},
            {396,   11880,  960000,     OMX_VIDEO_AVCLevel13,   OMX_VIDEO_AVCProfileHigh},
            {396,   11880,  2500000,    OMX_VIDEO_AVCLevel2,    OMX_VIDEO_AVCProfileHigh},
            {792,   19800,  5000000,    OMX_VIDEO_AVCLevel21,   OMX_VIDEO_AVCProfileHigh},
            {1620,  20250,  5000000,    OMX_VIDEO_AVCLevel22,   OMX_VIDEO_AVCProfileHigh},
            {1620,  40500,  12500000,   OMX_VIDEO_AVCLevel3,    OMX_VIDEO_AVCProfileHigh},
            {3600,  108000, 17500000,   OMX_VIDEO_AVCLevel31,   OMX_VIDEO_AVCProfileHigh},
            {5120,  216000, 25000000,   OMX_VIDEO_AVCLevel32,   OMX_VIDEO_AVCProfileHigh},
            {8192,  245760, 25000000,   OMX_VIDEO_AVCLevel4,    OMX_VIDEO_AVCProfileHigh}
        };
        static std::size_t const g_ProfileLevelsTableSizeH264HighQuality = sizeof(g_ProfileLevelsTableH264HighQuality)/sizeof(ProfileLevel);



    }

    char const *OMXCore::EXT_OMX_google_android_index_storeMetaDataInBuffers = "OMX.google.android.index.storeMetaDataInBuffers";
    char const *OMXCore::EXT_OMX_google_android_index_prependSPSPPSToIDRFrames = "OMX.google.android.index.prependSPSPPSToIDRFrames";


    OMXCore::OMXCore()
    {
        m_bInitialized = (OMX_ErrorNone == OMX_Init());
    }

    OMXCore::~OMXCore()
    {
        OMX_Deinit();
        m_bInitialized = false;
    }

    std::size_t OMXCore::numComponents() const
    {
        return SIZE_OF_CORE;
    }

    system::error_code OMXCore::getComponentName(std::size_t index, std::string &name) const
    {
        if(index >= SIZE_OF_CORE)
            return base::makeErrorCode(base::kEOutOfRange);
        name = ::core[index].name;
        return base::makeErrorCode(base::kENoError);
    }

    system::error_code OMXCore::getComponentRoles(std::size_t index, std::vector<std::string> &roles) const
    {
        if(index >= SIZE_OF_CORE)
            return base::makeErrorCode(base::kEOutOfRange);
        roles.clear();
        for(int r=0; r<OMX_CORE_MAX_CMP_ROLES && ::core[index].roles[r]; ++r)
            roles.push_back(std::string(::core[index].roles[r]));
        return base::makeErrorCode(base::kENoError);
    }

    system::error_code OMXCore::createComponentHandle(OMX_HANDLETYPE &handle, std::size_t index, OMX_PTR data, OMX_CALLBACKTYPE *callbacks) const
    {
        handle = 0;
        ::core[index].fn_ptr = omx_core_load_cmp_library(::core[index].so_lib_name, &::core[index].so_lib_handle);


        if(::core[index].fn_ptr)
        {
            // Construct the component requested
            // Function returns the opaque handle
            void* pComp = (*(::core[index].fn_ptr))();
            if(pComp)
            {
                void *hComp = NULL;
                hComp = ::qc_omx_create_component_wrapper((OMX_PTR)pComp);

                if((::qc_omx_component_init(hComp, ::core[index].name)) != OMX_ErrorNone)
                {
                    return base::makeErrorCode(base::kENotFound);
                }
                ::qc_omx_component_set_callbacks(hComp, callbacks, data);

                int hnd_index = get_comp_handle_index(::core[index].name);
                if(hnd_index >= 0)
                {
                    ::core[index].inst[hnd_index] = handle = (OMX_HANDLETYPE) hComp;
                }
                else
                {
                    return base::makeErrorCode(base::kENotFound);
                }
            }
        }
        return base::makeErrorCode(base::kENoError);
    }

    system::error_code OMXCore::destroyComponentHandle(OMX_HANDLETYPE handle) const
    {
        int err = 0, i = 0;

        // 0. Check that we have an active instance
        if((i = is_cmp_handle_exists(handle)) >=0)
        {
            // 1. Delete the component
            if(::qc_omx_component_deinit(handle) == OMX_ErrorNone)
            {
                /* Unload component library */
                if(((unsigned int) i < SIZE_OF_CORE) && ::core[i].so_lib_handle)
                {
                    if(check_lib_unload(i))
                    {
                        err = ::dlclose(::core[i].so_lib_handle);
                        if(err)
                        {
                        }
                        ::core[i].so_lib_handle = NULL;
                    }
                }
                clear_cmp_handle(handle);
            }
            else
            {
                return base::makeErrorCode(base::kENotFound);
            }
        }
        return base::makeErrorCode(base::kENoError);
    }

    system::error_code OMXCore::registerVideoEncoders(ComponentFactory &f)
    {
        if(!isInitialized())
            return base::makeErrorCode(base::kENotFound);

        for(std::size_t c = 0; c < numComponents(); ++c)
        {

            std::string comp_name;
            if(getComponentName(c, comp_name))
                continue;

            if(algorithm::icontains(comp_name, "secure"))
                continue;

            std::vector<std::string> comp_roles;
            if(getComponentRoles(c, comp_roles))
                continue;

            for(std::size_t ir = 0; ir < comp_roles.size(); ++ir)
            {
                std::vector<std::string> comp_role_tokens;

                algorithm::split(comp_role_tokens, comp_roles[ir], algorithm::is_any_of("."), algorithm::token_compress_on);
                if(comp_role_tokens.size() != 2 || comp_role_tokens[0] != "video_encoder")
                    continue;

                eVideoCodecType videoCodecType = videoCodecTypeFromOMXRole(comp_role_tokens[1]);
                if(!videoCodecType)
                    continue;

                f.registerComponent(
                    intrusive_ptr<Component::Registry>(
                        new VideoEncoder::Registry(comp_name, videoCodecType, c)
                    )
                );
            }
        }

        return base::makeErrorCode(base::kENoError);
    }

    system::error_code OMXCore::registerVideoDecoders(ComponentFactory &f)
    {
        if(!isInitialized())
            return base::makeErrorCode(base::kENotFound);

        for(std::size_t c = 0; c < numComponents(); ++c)
        {

            std::string comp_name;
            if(getComponentName(c, comp_name))
                continue;

            if(algorithm::icontains(comp_name, "secure"))
                continue;

            std::vector<std::string> comp_roles;
            if(getComponentRoles(c, comp_roles))
                continue;

            for(std::size_t ir = 0; ir < comp_roles.size(); ++ir)
            {
                std::vector<std::string> comp_role_tokens;

                algorithm::split(comp_role_tokens, comp_roles[ir], algorithm::is_any_of("."), algorithm::token_compress_on);
                if(comp_role_tokens.size() != 2 || comp_role_tokens[0] != "video_decoder")
                    continue;

                eVideoCodecType videoCodecType = videoCodecTypeFromOMXRole(comp_role_tokens[1]);
                if(!videoCodecType)
                    continue;


                f.registerComponent(
                    intrusive_ptr<Component::Registry>(
                        new VideoDecoder::Registry(comp_name, videoCodecType, c)
                    )
                );
            }
        }
        return base::makeErrorCode(base::kENoError);
    }

    eVideoCodecType OMXCore::videoCodecTypeFromOMXRole(std::string const &name)
    {
        if(name == "mpeg4")
            return kVideoCodecTypeMPEG4;

        if(name == "h263")
            return kVideoCodecTypeH263;

        if(name == "avc")
            return kVideoCodecTypeH264;

        return kVideoCodecTypeInvalid;
    }

    OMX_VIDEO_CODINGTYPE OMXCore::omxCodingTypeFromVideoCodecType(eVideoCodecType vct)
    {
        switch(vct)
        {
        case kVideoCodecTypeMPEG4:  return OMX_VIDEO_CodingMPEG4;   break;
        case kVideoCodecTypeH263:   return OMX_VIDEO_CodingH263;    break;
        case kVideoCodecTypeH264:   return OMX_VIDEO_CodingAVC;     break;
        default: break;
        }
        return OMX_VIDEO_CodingUnused;
    }

    char const* OMXCore::h264ProfileName(OMX_VIDEO_AVCPROFILETYPE profile)
    {
        switch(profile)
        {
            case OMX_VIDEO_AVCProfileBaseline:          return "OMX_VIDEO_AVCProfileBaseline";              break;
            case OMX_VIDEO_AVCProfileMain:              return "OMX_VIDEO_AVCProfileMain";                  break;
            case OMX_VIDEO_AVCProfileExtended:          return "OMX_VIDEO_AVCProfileExtended";              break;
            case OMX_VIDEO_AVCProfileHigh:              return "OMX_VIDEO_AVCProfileHigh";                  break;
            case OMX_VIDEO_AVCProfileHigh10:            return "OMX_VIDEO_AVCProfileHigh10";                break;
            case OMX_VIDEO_AVCProfileHigh422:           return "OMX_VIDEO_AVCProfileHigh422";               break;
            case OMX_VIDEO_AVCProfileHigh444:           return "OMX_VIDEO_AVCProfileHigh444";               break;
            case OMX_VIDEO_AVCProfileKhronosExtensions: return "OMX_VIDEO_AVCProfileKhronosExtensions";     break;
            case OMX_VIDEO_AVCProfileVendorStartUnused: return "OMX_VIDEO_AVCProfileVendorStartUnused";     break;
            case OMX_VIDEO_AVCProfileMax:               return "OMX_VIDEO_AVCProfileMax";                   break;
        }
        return "Unknown!";
    }

    char const* OMXCore::h264LevelName(OMX_VIDEO_AVCLEVELTYPE level)
    {
        switch(level)
        {
        case OMX_VIDEO_AVCLevel1:                   return "OMX_VIDEO_AVCLevel1";                   break;
        case OMX_VIDEO_AVCLevel1b:                  return "OMX_VIDEO_AVCLevel1b";                  break;
        case OMX_VIDEO_AVCLevel11:                  return "OMX_VIDEO_AVCLevel11";                  break;
        case OMX_VIDEO_AVCLevel12:                  return "OMX_VIDEO_AVCLevel12";                  break;
        case OMX_VIDEO_AVCLevel13:                  return "OMX_VIDEO_AVCLevel13";                  break;
        case OMX_VIDEO_AVCLevel2:                   return "OMX_VIDEO_AVCLevel2";                   break;
        case OMX_VIDEO_AVCLevel21:                  return "OMX_VIDEO_AVCLevel21";                  break;
        case OMX_VIDEO_AVCLevel22:                  return "OMX_VIDEO_AVCLevel22";                  break;
        case OMX_VIDEO_AVCLevel3:                   return "OMX_VIDEO_AVCLevel3";                   break;
        case OMX_VIDEO_AVCLevel31:                  return "OMX_VIDEO_AVCLevel31";                  break;
        case OMX_VIDEO_AVCLevel32:                  return "OMX_VIDEO_AVCLevel32";                  break;
        case OMX_VIDEO_AVCLevel4:                   return "OMX_VIDEO_AVCLevel4";                   break;
        case OMX_VIDEO_AVCLevel41:                  return "OMX_VIDEO_AVCLevel41";                  break;
        case OMX_VIDEO_AVCLevel42:                  return "OMX_VIDEO_AVCLevel42";                  break;
        case OMX_VIDEO_AVCLevel5:                   return "OMX_VIDEO_AVCLevel5";                   break;
        case OMX_VIDEO_AVCLevel51:                  return "OMX_VIDEO_AVCLevel51";                  break;
        case OMX_VIDEO_AVCLevelKhronosExtensions:   return "OMX_VIDEO_AVCLevelKhronosExtensions";   break;
        case OMX_VIDEO_AVCLevelVendorStartUnused:   return "OMX_VIDEO_AVCLevelVendorStartUnused";   break;
        case OMX_VIDEO_AVCLevelMax:                 return "OMX_VIDEO_AVCLevelMax";                 break;
        }
        return "Unknown!";
    }


    ProfileLevel const* OMXCore::chooseOMXProfileLevel(eVideoCodecType vct, eCodecProfile cp, Vec2i const &fs, float fps, std::size_t br)
    {
        std::size_t mb_per_frame = ((fs(1)+15) >> 4) * ((fs(0)+15)>>4);
        std::size_t mb_per_sec = static_cast<std::size_t>(mb_per_frame*fps);

        ProfileLevel *table = 0;
        std::size_t table_size = 0;

        switch(vct)
        {
        case kVideoCodecTypeMPEG4:
            if(cp == kCodecProfileHighQuality)
            {
                table = g_ProfileLevelsTableMPEG4HighQuality;
                table_size = g_ProfileLevelsTableSizeMPEG4HighQuality;
            }
            else
            {
                table = g_ProfileLevelsTableMPEG4LowLatency;
                table_size = g_ProfileLevelsTableSizeMPEG4LowLatency;
            }
            break;

        case kVideoCodecTypeH263:
            table = g_ProfileLevelsTableH263;
            table_size = g_ProfileLevelsTableSizeH263;
            break;

        case kVideoCodecTypeH264:
            if(cp == kCodecProfileHighQuality)
            {
                table = g_ProfileLevelsTableH264HighQuality;
                table_size = g_ProfileLevelsTableSizeH264HighQuality;
            }
            else
            {
                table = g_ProfileLevelsTableH264LowLatency;
                table_size = g_ProfileLevelsTableSizeH264LowLatency;
            }
            break;

        default:
            break;
        }

        if(!table || !table_size)
            return 0;

        std::size_t best_match = 0;
        for(std::size_t pl = 1; pl < table_size; ++pl)
        {
            if( mb_per_frame <= table[pl].max_mb_per_frame &&
                mb_per_sec <= table[pl].max_mb_per_second &&
                br <= table[pl].max_bitrate
            )
            {
                best_match = pl;
                break;
            }
        }
        return table + best_match;
    }

} //namespace omx
} //namespace media
} //namespace uquad
