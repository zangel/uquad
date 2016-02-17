#ifndef UQUAD_BASE_PIXEL_TRAITS_H
#define UQUAD_BASE_PIXEL_TRAITS_H

#include "Config.h"
#include "PixelFormat.h"

namespace uquad
{
namespace base
{
	template <ePixelFormat> struct PixelTraits {};
    
    template <>
    struct PixelTraits<kPixelFormatRGB8>
    {
        typedef gil::rgb8_pixel_t pixel_t;
        typedef gil::image<pixel_t> image_t;
        typedef image_t::view_t view_t;
        static std::size_t const num_planes = 1;
    };
    
    template <>
    struct PixelTraits<kPixelFormatBGRA8>
    {
        typedef gil::bgra8_pixel_t pixel_t;
        typedef gil::image<pixel_t> image_t;
        typedef image_t::view_t view_t;
        static std::size_t const num_planes = 1;
    };

    template <>
    struct PixelTraits<kPixelFormatYUV420sp>
    {
        typedef gil::ycbcr_601_8_pixel_t pixel_t;
        static bool const uv_swap = false;
        typedef gil::yuv420sp_image
        <
            pixel_t,
            uv_swap
        > image_t;
        typedef image_t::view_t view_t;
        static std::size_t const num_planes = 1;
    };
    

} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_PIXEL_TRAITS_H
