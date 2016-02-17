#include "VT.h"
#include "H264VideoEncoder.h"
#include "H264VideoDecoder.h"
#include "../../base/Error.h"

namespace uquad
{
namespace media
{
namespace vt
{
    namespace
    {
        static ProfileLevel g_ProfileLevelsTableMPEG4LowLatency[] =
        {
            {99,    1485,   64000,      kVTProfileLevel_MP4V_Simple_L0          },
            {396,   5940,   128000,     kVTProfileLevel_MP4V_Simple_L1          },
            {396,   11880,  384000,     kVTProfileLevel_MP4V_Simple_L2          },
            {1200,  36000,  4000000,    kVTProfileLevel_MP4V_Simple_L3          },
            {1620,  40500,  8000000,    kVTProfileLevel_MP4V_Simple_L3          },
            {3600,  108000, 12000000,   kVTProfileLevel_MP4V_Simple_L3          }
        };
        static std::size_t const g_ProfileLevelsTableSizeMPEG4LowLatency = sizeof(g_ProfileLevelsTableMPEG4LowLatency)/sizeof(ProfileLevel);

        static ProfileLevel g_ProfileLevelsTableMPEG4HighQuality[] =
        {
            {99,    1485,   128000,     kVTProfileLevel_MP4V_AdvancedSimple_L0  },
            {396,   5940,   384000,     kVTProfileLevel_MP4V_AdvancedSimple_L1  },
            {396,   11880,  768000,     kVTProfileLevel_MP4V_AdvancedSimple_L2  },
            {792,   23760,  3000000,    kVTProfileLevel_MP4V_AdvancedSimple_L3  },
            {1620,  48600,  8000000,    kVTProfileLevel_MP4V_AdvancedSimple_L4  }
        };
        static std::size_t const g_ProfileLevelsTableSizeMPEG4HighQuality = sizeof(g_ProfileLevelsTableMPEG4HighQuality)/sizeof(ProfileLevel);


        static ProfileLevel g_ProfileLevelsTableH263[] =
        {
            {99,    1485,   64000,      kVTProfileLevel_H263_Profile0_Level10       },
            {396,   5940,   128000,     kVTProfileLevel_H263_Profile0_Level10       },
            {396,   11880,  384000,     kVTProfileLevel_H263_Profile0_Level10       },
            {396,   11880,  2048000,    kVTProfileLevel_H263_Profile0_Level45       },
            {99,    1485,   128000,     kVTProfileLevel_H263_Profile0_Level45       },
            {396,   19800,  4096000,    kVTProfileLevel_H263_Profile0_Level45       },
            {810,   40500,  8192000,    kVTProfileLevel_H263_Profile3_Level45       },
            {1620,  81000,  16384000,   kVTProfileLevel_H263_Profile3_Level45       }
        };
        static std::size_t const g_ProfileLevelsTableSizeH263 = sizeof(g_ProfileLevelsTableH263)/sizeof(ProfileLevel);
        
        static ProfileLevel g_ProfileLevelsTableH264LowLatency[] =
        {
            {99,    1485,   64000,      kVTProfileLevel_H264_Baseline_1_3           },
            {99,    1485,   128000,     kVTProfileLevel_H264_Baseline_1_3           },
            {396,   3000,   192000,     kVTProfileLevel_H264_Baseline_1_3           },
            {396,   6000,   384000,     kVTProfileLevel_H264_Baseline_3_0           },
            {396,   11880,  768000,     kVTProfileLevel_H264_Baseline_3_1           },
            {396,   11880,  2000000,    kVTProfileLevel_H264_Baseline_3_2           },
            {792,   19800,  4000000,    kVTProfileLevel_H264_Baseline_4_0           },
            {1620,  20250,  4000000,    kVTProfileLevel_H264_Baseline_4_1           },
            {1620,  40500,  10000000,   kVTProfileLevel_H264_Baseline_4_2           },
            {3600,  108000, 14000000,   kVTProfileLevel_H264_Baseline_5_0           },
            {5120,  216000, 20000000,   kVTProfileLevel_H264_Baseline_5_1           },
            {8192,  245760, 20000000,   kVTProfileLevel_H264_Baseline_5_2           }
        };
        static std::size_t const g_ProfileLevelsTableSizeH264LowLatency = sizeof(g_ProfileLevelsTableH264LowLatency)/sizeof(ProfileLevel);

        static ProfileLevel g_ProfileLevelsTableH264HighQuality[] =
        {
            {99,    1485,   64000,      kVTProfileLevel_H264_High_3_0               },
            {99,    1485,   160000,     kVTProfileLevel_H264_High_3_0               },
            {396,   3000,   240000,     kVTProfileLevel_H264_High_3_0               },
            {396,   6000,   480000,     kVTProfileLevel_H264_High_3_0               },
            {396,   11880,  960000,     kVTProfileLevel_H264_High_3_1               },
            {396,   11880,  2500000,    kVTProfileLevel_H264_High_3_2               },
            {792,   19800,  5000000,    kVTProfileLevel_H264_High_4_0               },
            {1620,  20250,  5000000,    kVTProfileLevel_H264_High_4_1               },
            {1620,  40500,  12500000,   kVTProfileLevel_H264_High_4_2               },
            {3600,  108000, 17500000,   kVTProfileLevel_H264_High_5_0               },
            {5120,  216000, 25000000,   kVTProfileLevel_H264_High_5_1               },
            {8192,  245760, 25000000,   kVTProfileLevel_H264_High_5_2               }
        };
        static std::size_t const g_ProfileLevelsTableSizeH264HighQuality = sizeof(g_ProfileLevelsTableH264HighQuality)/sizeof(ProfileLevel);
    }
    
    VT::VT()
        : m_bInitialized(false)
    {
        m_bInitialized = true;
    }
    
    VT::~VT()
    {
    }
    
    std::string VT::videoCodecNameFromEncoderSpec(CFDictionaryRef es)
    {
        char strEncoderName[256] = { '\0' };
        CFStringRef vtEncoderName = (CFStringRef)CFDictionaryGetValue(es, kVTVideoEncoderList_EncoderName);
        if(!vtEncoderName)
            return std::string();
        
        if(!CFStringGetCString(vtEncoderName, strEncoderName, 256, kCFStringEncodingUTF8))
            return std::string();
        
        return std::string(strEncoderName);
    }
    
    eVideoCodecType VT::videoCodecTypeFromEncoderSpec(CFDictionaryRef es)
    {
        CFNumberRef vtEncoderType = (CFNumberRef)CFDictionaryGetValue(es, kVTVideoEncoderList_CodecType);
        if(!vtEncoderType)
            return kVideoCodecTypeInvalid;
        
        SInt32 encoderType;
        if(!CFNumberGetValue(vtEncoderType, kCFNumberSInt32Type, &encoderType))
            return kVideoCodecTypeInvalid;
        
        switch(encoderType)
        {
        case kCMVideoCodecType_H263:        return kVideoCodecTypeH263;     break;
        case kCMVideoCodecType_H264:        return kVideoCodecTypeH264;     break;
        case kCMVideoCodecType_MPEG4Video:  return kVideoCodecTypeMPEG4;    break;
        default: break;
        }
        
        return kVideoCodecTypeInvalid;
    }
    
    CMVideoCodecType VT::cmVideoCodecTypeFromVideoCodecType(eVideoCodecType vct)
    {
        switch(vct)
        {
        case kVideoCodecTypeH263:           return kCMVideoCodecType_H263;  break;
        case kVideoCodecTypeH264:           return kCMVideoCodecType_H264;  break;
        case kVideoCodecTypeMPEG4:          return kCMVideoCodecType_MPEG4Video;  break;
        default: break;
        }
        return  CMVideoCodecType(0);
    }
    
    ProfileLevel const* VT::chooseVTProfileLevel(eVideoCodecType vct, eCodecProfile cp, Vec2i const &fs, float fps, std::size_t br)
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
    
    system::error_code VT::registerVideoEncoders(ComponentFactory &f)
    {
        if(!isInitialized())
			return base::makeErrorCode(base::kENotFound);
            
        CFArrayRef vtEncoderList = 0;
        if(VTCopyVideoEncoderList(0, &vtEncoderList) == noErr)
        {
            for(std::size_t ie = 0; ie < CFArrayGetCount(vtEncoderList); ++ie)
            {
                CFDictionaryRef vtEncoderDict = (CFDictionaryRef)CFArrayGetValueAtIndex(vtEncoderList, ie);
                if(!vtEncoderDict)
                    continue;
                
                eVideoCodecType vct = videoCodecTypeFromEncoderSpec(vtEncoderDict);
                if(!vct)
                    continue;
                
                switch(vct)
                {
                case kVideoCodecTypeH264:
                    {
                        f.registerComponent(
                            intrusive_ptr<Component::Registry>(
                                new H264VideoEncoder::Registry(
                                    videoCodecNameFromEncoderSpec(vtEncoderDict),
                                    vtEncoderDict
                                )
                            )
                        );
                    }
                    break;
                    
                default:
                    break;
                }
                
                
            }
            CFRelease(vtEncoderList);
        }
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code VT::registerVideoDecoders(ComponentFactory &f)
    {
        if(!isInitialized())
			return base::makeErrorCode(base::kENotFound);
        
        f.registerComponent(
            intrusive_ptr<Component::Registry>(new H264VideoDecoder::Registry("com.apple.VideoToolbox.H264"))
        );
        return base::makeErrorCode(base::kENoError);
    }
    
} //namespace vt
} //namespace media
} //namespace uquad