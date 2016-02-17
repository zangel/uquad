#include <boost/test/unit_test.hpp>
#include <uquad/base/Image.h>

#include <boost/gil/extension/io/jpeg_dynamic_io.hpp>
#include <boost/gil/extension/yuv420sp/yuv420sp.hpp>


using namespace uquad;

BOOST_AUTO_TEST_CASE(Image)
{
    typedef gil::yuv420sp_image<gil::ycbcr_601_8_pixel_t, false> yuv_image_t;
    typedef yuv_image_t::pixel_t yuv_pixel_t;
    typedef yuv_image_t::view_t yuv_view_t;
    
    
    yuv_image_t yuv_img(640,480);
    
    yuv_pixel_t fill_color;
    gil::color_convert(gil::rgb8_pixel_t(255,0,0), fill_color);
    gil::fill_pixels(gil::view(yuv_img), fill_color);
    gil::jpeg_write_view("yuv_img.jpg", gil::color_converted_view<gil::rgb8_pixel_t>(gil::view(yuv_img)));
    
    
    base::PixelTraits<base::kPixelFormatYUV420sp>::image_t img(640, 480);
    //gil::fill_pixels( gil::view(img), base::PixelTraits<base::kPFMT_RGB8>::pixel_t(10, 20, 30));
    
    //gil::fill_pixels( gil::view(img), red_color);
    gil::fill_pixels(base::Image::view_t(gil::view(img)), fill_color);
    
    base::PixelTraits<base::kPixelFormatRGB8>::image_t img_converted(640, 480);
    //gil::copy_and_convert_pixels(base::Image::view_t(gil::view(img)), gil::view(img_converted));
    gil::jpeg_write_view("img.jpg", gil::color_converted_view<base::PixelTraits<base::kPixelFormatRGB8>::pixel_t>(gil::view(img)));
    
}
