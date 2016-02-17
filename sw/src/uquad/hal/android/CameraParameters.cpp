#include "CameraParameters.h"
#include "../../base/Error.h"

#if defined(UQUAD_PLATFORM_Mac)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunsequenced"
#endif

#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/repository/include/qi_kwd.hpp>
#include <boost/spirit/repository/include/qi_keywords.hpp>


namespace uquad
{
namespace hal
{
namespace android
{
    namespace
    {
        namespace spirit = boost::spirit;
        namespace phoenix = boost::phoenix;
        namespace ascii = spirit::ascii;
        namespace qi
        {
            using namespace spirit::qi;
            using namespace spirit::repository::qi;
        }
        
        namespace karma
        {
            using namespace spirit::karma;
        }
        
        template <typename Iterator>
        struct parser : qi::grammar<Iterator, ascii::space_type>
        {
            parser(CameraParameters &p)
                : parser::base_type(r_params)
                , params(p)
            {
                r_params = r_param % ';';
                r_param =
                    r_preview_format[phoenix::ref(params.previewFormat)=qi::_1] |
                    r_preview_format_values[phoenix::ref(params.supportedPreviewFormats)=qi::_1] |
                    r_preview_size[phoenix::ref(params.previewSize)=qi::_1] |
                    r_preview_size_values[phoenix::ref(params.supportedPreviewSizes)=qi::_1] |
                    r_preview_frame_rate[phoenix::ref(params.previewFPS)=qi::_1] |
                    r_preview_frame_rate_values[phoenix::ref(params.supportedPreviewFPS)=qi::_1] |
                    r_preview_fps_range[phoenix::ref(params.previewFPSRange)=qi::_1] |
                    r_preview_fps_range_values[phoenix::ref(params.supportedPreviewFPSRange)=qi::_1] |
                    r_video_format[phoenix::ref(params.videoFormat)=qi::_1] |
                    r_video_size[phoenix::ref(params.videoSize)=qi::_1] |
                    r_video_size_values[phoenix::ref(params.supportedVideoSizes)=qi::_1] |
                    r_picture_format[phoenix::ref(params.pictureFormat)=qi::_1] |
                    r_picture_format_values[phoenix::ref(params.supportedPictureFormats)=qi::_1] |
                    r_picture_size[phoenix::ref(params.pictureSize)=qi::_1] |
                    r_picture_size_values[phoenix::ref(params.supportedPictureSizes)=qi::_1] |
                    r_no_display_mode[phoenix::ref(params.noDisplayMode)=qi::_1] |
					r_preview_flip[phoenix::ref(params.previewFlip)=qi::_1] |
					r_video_flip[phoenix::ref(params.videoFlip)=qi::_1] |
                    r_focus_mode[phoenix::ref(params.focusMode)=qi::_1] |
                    r_key_value[phoenix::insert(phoenix::ref(params.unused), qi::_1)];
                
                r_preview_format = qi::lit("preview-format") >> "=" >> r_pixel_format;
                r_preview_format_values = qi::lit("preview-format-values") >> "=" >> r_pixel_format_vector;
                r_preview_size = qi::lit("preview-size") >> "=" >> r_Vec2i;
                r_preview_size_values = qi::lit("preview-size-values") >> "=" >> r_Vec2i_vector;
                r_preview_frame_rate = qi::lit("preview-frame-rate") >> "=" >> qi::float_;
                r_preview_frame_rate_values = qi::lit("preview-frame-rate-values") >> "=" >> r_float_vector;
                r_preview_fps_range = qi::lit("preview-fps-range") >> "=" >> r_FPSRange;
                r_preview_fps_range_values = qi::lit("preview-fps-range-values") >> "=" >> "(" >> r_FPSRange >> ")";
                
                r_video_format = qi::lit("video-frame-format") >> "=" >> r_pixel_format;
                r_video_size = qi::lit("video-size") >> "=" >> r_Vec2i;
                r_video_size_values = qi::lit("video-size-values") >> "=" >> r_Vec2i_vector;

                r_picture_format = qi::lit("picture-format") >> "=" >> r_image_format;
                r_picture_format_values = qi::lit("picture-format-values") >> "=" >> r_image_format_vector;
                r_picture_size = qi::lit("picture-size") >> "=" >> r_Vec2i;
                r_picture_size_values = qi::lit("picture-size-values") >> "=" >> r_Vec2i_vector;
                r_no_display_mode = qi::lit("no-display-mode") >> "=" >> r_bool_from_int;
                
                r_preview_flip = qi::lit("preview-flip") >> "=" >> r_eFlipMode;
                r_video_flip = qi::lit("video-flip") >> "=" >> r_eFlipMode;
                
                r_focus_mode = qi::lit("focus-mode") >> "=" >> r_eFocusMode;

                r_pixel_format_vector = r_pixel_format % ',';
                r_pixel_format.add("yuv420sp", base::kPixelFormatYUV420sp);
                r_pixel_format.add("yuv420sp-adreno", base::kPixelFormatYUV420spAdreno);
                r_pixel_format.add("yuv420p", base::kPixelFormatYUV420p);
                r_pixel_format.add("nv12", base::kPixelFormatYUV420NV12);
                
                r_image_format_vector = r_image_format % ',';
                r_image_format.add("jpeg", base::kImageFormatJPEG);
                r_image_format.add("raw", base::kImageFormatRAW);
                
                r_Vec2i_vector = r_Vec2i % ',';
                
                r_Vec2i = (qi::int_ >> "x" >> qi::int_)
                    [
                        phoenix::ref(qi::_val)[0] = qi::_1,
                        phoenix::ref(qi::_val)[1] = qi::_2
                    ];
                
                r_FPSRange = (qi::int_ >> "," >> qi::int_)
                    [
                        phoenix::ref(qi::_val)[0] = qi::_1,
                        phoenix::ref(qi::_val)[1] = qi::_2
                    ];
                
                r_float_vector = qi::float_ % ',';
                
                r_bool_from_int = qi::lit("0")[qi::_val = false] | qi::lit("1")[qi::_val = true];
                
                r_eFlipMode.add("0", CameraParameters::kFlipModeNone);
                r_eFlipMode.add("1", CameraParameters::kFlipModeHorizontal);
                r_eFlipMode.add("2", CameraParameters::kFlipModeVertical);
                r_eFlipMode.add("3", CameraParameters::kFlipModeHorizontalAndVertical);
                
                r_eFocusMode.add("auto", CameraParameters::kFocusModeAuto);
                r_eFocusMode.add("infinity", CameraParameters::kFocusModeInfinity);
                r_eFocusMode.add("normal", CameraParameters::kFocusModeNormal);
                r_eFocusMode.add("macro", CameraParameters::kFocusModeMacro);
                r_eFocusMode.add("continuous-picture", CameraParameters::kFocusModeContinuousPicture);
                r_eFocusMode.add("continuous-video", CameraParameters::kFocusModeContinuousVideo);
                
                
                r_key_value = (r_key >> "=" >> r_value)
                    [
                        phoenix::bind(&std::pair<std::string,std::string>::first, qi::_val) = qi::_1,
                        phoenix::bind(&std::pair<std::string,std::string>::second, qi::_val) = qi::_2
                    ];
                
                r_key = qi::raw[+(qi::char_ - '=')];
                r_value = qi::raw[*(qi::char_ - ';')];
            }
            
            CameraParameters &params;
            qi::rule<Iterator, ascii::space_type> r_params, r_param;
            qi::rule<Iterator, base::ePixelFormat(), ascii::space_type> r_preview_format;
            qi::rule<Iterator, unordered_set<base::ePixelFormat>(), ascii::space_type> r_preview_format_values;
            qi::rule<Iterator, Vec2i(), ascii::space_type> r_preview_size;
            qi::rule<Iterator, unordered_set<Vec2i>(), ascii::space_type> r_preview_size_values;
            qi::rule<Iterator, float(), ascii::space_type> r_preview_frame_rate;
            qi::rule<Iterator, unordered_set<float>(), ascii::space_type> r_preview_frame_rate_values;
            qi::rule<Iterator, Vec2i(), ascii::space_type> r_preview_fps_range;
            qi::rule<Iterator, Vec2i(), ascii::space_type> r_preview_fps_range_values;
            qi::rule<Iterator, base::ePixelFormat(), ascii::space_type> r_video_format;
            qi::rule<Iterator, Vec2i(), ascii::space_type> r_video_size;
            qi::rule<Iterator, unordered_set<Vec2i>(), ascii::space_type> r_video_size_values;
            qi::rule<Iterator, base::eImageFormat(), ascii::space_type> r_picture_format;
            qi::rule<Iterator, unordered_set<base::eImageFormat>(), ascii::space_type> r_picture_format_values;
            qi::rule<Iterator, Vec2i(), ascii::space_type> r_picture_size;
            qi::rule<Iterator, unordered_set<Vec2i>(), ascii::space_type> r_picture_size_values;
            qi::rule<Iterator, bool(), ascii::space_type> r_no_display_mode;
            qi::rule<Iterator, CameraParameters::eFlipMode(), ascii::space_type> r_preview_flip;
            qi::rule<Iterator, CameraParameters::eFlipMode(), ascii::space_type> r_video_flip;
            qi::rule<Iterator, CameraParameters::eFocusMode(), ascii::space_type> r_focus_mode;
            qi::symbols<char, base::ePixelFormat> r_pixel_format;
            qi::rule<Iterator, unordered_set<base::ePixelFormat>(), ascii::space_type> r_pixel_format_vector;
            qi::symbols<char, base::eImageFormat> r_image_format;
            qi::rule<Iterator, unordered_set<base::eImageFormat>(), ascii::space_type> r_image_format_vector;
            qi::rule<Iterator, unordered_set<Vec2i>(), ascii::space_type> r_Vec2i_vector;
            qi::rule<Iterator, Vec2i(), ascii::space_type> r_Vec2i;
            qi::rule<Iterator, Vec2i(), ascii::space_type> r_FPSRange;
            qi::rule<Iterator, unordered_set<float>(), ascii::space_type> r_float_vector;
            qi::rule<Iterator, bool(), ascii::space_type> r_bool_from_int;
            qi::symbols<char, CameraParameters::eFlipMode> r_eFlipMode;
            qi::symbols<char, CameraParameters::eFocusMode> r_eFocusMode;
            qi::rule<Iterator, std::pair<std::string, std::string>(), ascii::space_type> r_key_value;
            qi::rule<Iterator, std::string(), ascii::space_type> r_key, r_value;
        };
        
        template <typename Iterator>
        struct generator
            : public karma::grammar<Iterator>
        {
            generator(CameraParameters &p)
                : generator::base_type(r_params)
                , params(p)
            {
                //r_params = karma::delimit(";")
                r_params =
                    r_preview_params <<
                    r_video_params <<
                    r_picture_params <<
                    r_general_params <<
                    r_unused[karma::_1 = phoenix::ref(params.unused)];
                
                
                r_preview_params = karma::delimit(";")
                    [
                        (-r_preview_format)[karma::_1 = phoenix::ref(params.previewFormat)] <<
                        (-r_preview_size)[karma::_1 = phoenix::ref(params.previewSize)] <<
                        (-r_preview_frame_rate)[karma::_1 = phoenix::ref(params.previewFPS)] <<
                        (-r_preview_fps_range)[karma::_1 = phoenix::ref(params.previewFPSRange)] <<
                        (-r_preview_flip)[karma::_1 = phoenix::ref(params.previewFlip)] <<
                        (-r_no_display_mode)[karma::_1 = phoenix::ref(params.noDisplayMode)]
                    ];
                
                r_video_params = karma::delimit(";")
                    [
                        (-r_video_format)[karma::_1 = phoenix::ref(params.videoFormat)] <<
                        (-r_video_size)[karma::_1 = phoenix::ref(params.videoSize)] <<
                        (-r_video_flip)[karma::_1 = phoenix::ref(params.videoFlip)]
                    ];
                
                r_picture_params = karma::delimit(";")
                    [
                        (-r_picture_format)[karma::_1 = phoenix::ref(params.pictureFormat)] <<
                        (-r_picture_size)[karma::_1 = phoenix::ref(params.pictureSize)]
                    ];
                
                r_general_params = karma::delimit(";")
                    [
                        (-r_focus_mode)[karma::_1 = phoenix::ref(params.focusMode)]
                    ];
                
                r_preview_format = karma::lit("preview-format") << "=" << r_pixel_format;
                r_preview_size = karma::lit("preview-size") << "=" << r_Vec2i;
                r_preview_frame_rate = karma::lit("preview-frame-rate") << "=" << karma::int_;
                r_preview_fps_range = karma::lit("preview-fps-range") << "=" << r_FPSRange;
                
                r_video_format = karma::lit("video-frame-format") << "=" << r_pixel_format;
                r_video_size = karma::lit("video-size") << "=" << r_Vec2i;

                r_picture_format = karma::lit("picture-format") << "=" << r_image_format;
                r_picture_size = karma::lit("picture-size") << "=" << r_Vec2i;
                r_no_display_mode = karma::lit("no-display-mode") << "=" << r_bool_as_int;
                r_preview_flip = karma::lit("preview-flip") << "=" << r_eFlipMode;
                r_video_flip = karma::lit("video-flip") << "=" << r_eFlipMode;
                
                r_focus_mode = karma::lit("focus-mode") << "=" << r_eFocusMode;
                
                r_pixel_format.add(base::kPixelFormatYUV420sp, "yuv420sp");
                r_pixel_format.add(base::kPixelFormatYUV420spAdreno, "yuv420sp-adreno");
                r_pixel_format.add(base::kPixelFormatYUV420p, "yuv420p");
                r_pixel_format.add(base::kPixelFormatYUV420NV12, "nv12");
                
                r_image_format.add(base::kImageFormatJPEG, "jpeg");
                r_image_format.add(base::kImageFormatRAW, "raw");
                
                r_Vec2i = karma::int_[karma::_1 = phoenix::ref(karma::_val)[0]] << "x" << karma::int_[karma::_1 = phoenix::ref(karma::_val)[1]];
                
                r_FPSRange = karma::int_[karma::_1 = phoenix::ref(karma::_val)[0]] << "," << karma::int_[karma::_1 = phoenix::ref(karma::_val)[1]];
                
                r_bool_as_int.add(false, "0");
                r_bool_as_int.add(true, "1");
                
                r_eFlipMode.add(CameraParameters::kFlipModeNone, "0");
                r_eFlipMode.add(CameraParameters::kFlipModeHorizontal, "1");
                r_eFlipMode.add(CameraParameters::kFlipModeVertical, "2");
                r_eFlipMode.add(CameraParameters::kFlipModeHorizontalAndVertical, "3");
                
                r_eFocusMode.add(CameraParameters::kFocusModeAuto, "auto");
                r_eFocusMode.add(CameraParameters::kFocusModeInfinity, "infinity");
                r_eFocusMode.add(CameraParameters::kFocusModeNormal, "normal");
                r_eFocusMode.add(CameraParameters::kFocusModeMacro, "macro");
                r_eFocusMode.add(CameraParameters::kFocusModeContinuousPicture, "continuous-picture");
                r_eFocusMode.add(CameraParameters::kFocusModeContinuousVideo, "continuous-video");
                
                r_unused = r_key_value % ";";
                
                
                r_key_value = (karma::string << "=" << karma::string)
                    [
                        karma::_1 = phoenix::bind(&std::pair<std::string,std::string>::first, karma::_val),
                        karma::_2 = phoenix::bind(&std::pair<std::string,std::string>::second, karma::_val)
                    ];
                
                
            }
        
            CameraParameters &params;
            karma::rule<Iterator> r_params, r_preview_params, r_video_params, r_picture_params, r_general_params;
            karma::rule<Iterator, base::ePixelFormat()> r_preview_format;
            karma::rule<Iterator, Vec2i()> r_preview_size;
            karma::rule<Iterator, float()> r_preview_frame_rate;
            karma::rule<Iterator, Vec2i()> r_preview_fps_range;
            karma::rule<Iterator, base::ePixelFormat()> r_video_format;
            karma::rule<Iterator, Vec2i()> r_video_size;
            karma::rule<Iterator, base::eImageFormat()> r_picture_format;
            karma::rule<Iterator, Vec2i()> r_picture_size;
            karma::rule<Iterator, bool()> r_no_display_mode;
            karma::rule<Iterator, CameraParameters::eFlipMode()> r_preview_flip;
            karma::rule<Iterator, CameraParameters::eFlipMode()> r_video_flip;
            karma::rule<Iterator, CameraParameters::eFocusMode()> r_focus_mode;
            karma::symbols<base::ePixelFormat, const char *> r_pixel_format;
            karma::symbols<base::eImageFormat, const char *> r_image_format;
            karma::rule<Iterator, Vec2i()> r_Vec2i;
            karma::rule<Iterator, Vec2i()> r_FPSRange;
            karma::symbols<bool, const char *> r_bool_as_int;
            karma::symbols<CameraParameters::eFlipMode, const char *> r_eFlipMode;
            karma::symbols<CameraParameters::eFocusMode, const char *> r_eFocusMode;
            karma::rule<Iterator, std::map<std::string, std::string>()> r_unused;
            karma::rule<Iterator, std::pair<std::string, std::string>()> r_key_value;
        };
        
        
        
    } //namespace
    
	CameraParameters::CameraParameters()
		: supportedPreviewFormats()
		, previewFormat()
		, supportedPreviewSizes()
		, previewSize()
		, supportedPreviewFPS()
		, previewFPS()
        , supportedPreviewFPSRange()
        , previewFPSRange()
        , noDisplayMode()
		, previewFlip()
        , videoFormat()
        , supportedVideoSizes()
        , videoSize()
        , videoFlip()
		, supportedPictureFormats()
		, pictureFormat()
		, supportedPictureSizes()
		, pictureSize()
        , focusMode()
        , unused()
	{
	}

	CameraParameters::CameraParameters(CameraParameters const &that)
		: supportedPreviewFormats(that.supportedPreviewFormats)
        , previewFormat(that.previewFormat)
		, supportedPreviewSizes(that.supportedPreviewSizes)
		, previewSize(that.previewSize)
		, supportedPreviewFPS(that.supportedPreviewFPS)
		, previewFPS(that.previewFPS)
        , supportedPreviewFPSRange(that.supportedPreviewFPSRange)
        , previewFPSRange(that.previewFPSRange)
        , noDisplayMode(that.noDisplayMode)
		, previewFlip(that.previewFlip)
        , videoFormat(that.videoFormat)
        , supportedVideoSizes(that.supportedVideoSizes)
        , videoSize(that.videoSize)
        , videoFlip(that.videoFlip)
		, supportedPictureFormats(that.supportedPictureFormats)
		, pictureFormat(that.pictureFormat)
		, supportedPictureSizes(that.supportedPictureSizes)
		, pictureSize(that.pictureSize)
        , focusMode(that.focusMode)
        , unused(that.unused)
	{

	}

	CameraParameters::~CameraParameters()
	{
	}

	CameraParameters& CameraParameters::operator =(CameraParameters const &rhs)
	{
		supportedPreviewFormats = rhs.supportedPreviewFormats;
		previewFormat = rhs.previewFormat;
		supportedPreviewSizes = rhs.supportedPreviewSizes;
		previewSize = rhs.previewSize;
		supportedPreviewFPS = rhs.supportedPreviewFPS;
		previewFPS = rhs.previewFPS;
        supportedPreviewFPSRange = rhs.supportedPreviewFPSRange;
        previewFPSRange = rhs.previewFPSRange;
        noDisplayMode = rhs.noDisplayMode;
        previewFlip = rhs.previewFlip;
        videoFormat = rhs.videoFormat;
        supportedVideoSizes = rhs.supportedVideoSizes;
        videoSize = rhs.videoSize;
        videoFlip = rhs.videoFlip;
        supportedPictureFormats = rhs.supportedPictureFormats;
		pictureFormat = rhs.pictureFormat;
		supportedPictureSizes = rhs.supportedPictureSizes;
		pictureSize = rhs.pictureSize;
        focusMode = rhs.focusMode;
        unused = rhs.unused;
		return *this;
	}

	bool CameraParameters::isValid() const
	{
		return
			supportedPreviewFormats &&
			supportedPreviewSizes &&
			supportedPreviewFPS &&
			previewFormat &&
			previewSize &&
			previewFPS &&
            previewFPSRange &&
            noDisplayMode &&
			videoFormat &&
            supportedVideoSizes &&
            videoSize &&
			supportedPictureFormats &&
			supportedPictureSizes &&
			pictureFormat &&
			pictureSize;
	}
    
    void CameraParameters::reset()
    {
        supportedPreviewFormats.reset();
        previewFormat.reset();
        supportedPreviewSizes.reset();
        previewSize.reset();
		supportedPreviewFPS.reset();
		previewFPS.reset();
        supportedPreviewFPSRange.reset();
        previewFPSRange.reset();
        noDisplayMode.reset();
        previewFlip.reset();
        videoFormat.reset();
        supportedVideoSizes.reset();
        videoSize.reset();
        videoFlip.reset();
        supportedPictureFormats.reset();
        pictureFormat.reset();
		supportedPictureSizes.reset();
        pictureSize.reset();
        focusMode.reset();
        unused.clear();
    }
    
    CameraParameters& CameraParameters::apply(CameraParameters const &rhs)
    {
        if(rhs.supportedPreviewFormats) supportedPreviewFormats = rhs.supportedPreviewFormats;
        if(rhs.previewFormat) previewFormat = rhs.previewFormat;
        if(rhs.supportedPreviewSizes) supportedPreviewSizes = rhs.supportedPreviewSizes;
        if(rhs.previewSize) previewSize = rhs.previewSize;
        if(rhs.supportedPreviewFPS) supportedPreviewFPS = rhs.supportedPreviewFPS;
        if(rhs.previewFPS) previewFPS = rhs.previewFPS;
        if(rhs.supportedPreviewFPSRange) supportedPreviewFPSRange = rhs.supportedPreviewFPSRange;
        if(rhs.previewFPSRange) previewFPSRange = rhs.previewFPSRange;
        if(rhs.noDisplayMode) noDisplayMode = rhs.noDisplayMode;
        if(rhs.previewFlip) previewFlip = rhs.previewFlip;
        if(rhs.videoFormat) videoFormat = rhs.videoFormat;
        if(rhs.supportedVideoSizes) supportedVideoSizes = rhs.supportedVideoSizes;
        if(rhs.videoSize) videoSize = rhs.videoSize;
        if(rhs.videoFlip) videoFlip = rhs.videoFlip;
        if(rhs.supportedPictureFormats) supportedPictureFormats = rhs.supportedPictureFormats;
        if(rhs.pictureFormat) pictureFormat = rhs.pictureFormat;
        if(rhs.supportedPictureSizes) supportedPictureSizes = rhs.supportedPictureSizes;
        if(rhs.pictureSize) pictureSize = rhs.pictureSize;
        if(rhs.focusMode) focusMode = rhs.focusMode;
        if(!rhs.unused.empty()) unused = rhs.unused;
        return *this;
    }

    void CameraParameters::removeOutOfRangeFPS()
    {
        if(supportedPreviewFPSRange && supportedPreviewFPS)
        {
            unordered_set<float>::iterator itSFPS = supportedPreviewFPS.get().begin();
            while(itSFPS != supportedPreviewFPS.get().end())
            {
                float fps = *itSFPS;
                if( fps < 1.0e-3f*supportedPreviewFPSRange.get()[0] ||
                    fps > 1.0e-3f*supportedPreviewFPSRange.get()[1])
                {
                    itSFPS = supportedPreviewFPS.get().erase(itSFPS);
                }
                else
                {
                    itSFPS++;
                }

            }
        }
    }

    system::error_code CameraParameters::parse(std::string const &s)
	{
        parser<std::string::const_iterator> paramsParser(*this);
        if(!qi::phrase_parse(
            s.begin(),
            s.end(),
            paramsParser,
            ascii::space
        ))
        {
            return base::makeErrorCode(base::kEInvalidContent);
        }

        return base::makeErrorCode(base::kENoError);
	}
    
    system::error_code CameraParameters::generate(std::string &is)
	{
        std::back_insert_iterator<std::string> i(is);
        generator< std::back_insert_iterator<std::string> > paramsGenerator(*this);
        if(!karma::generate(i, paramsGenerator))
        {
            return base::makeErrorCode(base::kEInvalidContent);
        }
        return base::makeErrorCode(base::kENoError);
	}

} //namespace android
} //namespace hal
} //namespace uquad

#if defined(UQUAD_PLATFORM_Mac)
#   pragma clang diagnostic pop
#endif
