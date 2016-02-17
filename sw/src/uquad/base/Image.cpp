#include "Image.h"
#include "Error.h"
#include "../utility/Constant.h"
#include <boost/gil/extension/io/jpeg_dynamic_io.hpp>


namespace uquad
{
namespace base
{
    Image::Image()
        : m_PixelBuffer()
    {
    }
    
    Image::Image(intrusive_ptr<PixelBuffer> const &pb)
        : m_PixelBuffer(pb)
    {
    }
    
    Image::~Image()
    {
    }
    
    bool Image::isValid() const
    {
        return m_PixelBuffer && m_PixelBuffer->isValid();
    }
    
    Vec2i Image::getSize() const
    {
        return isValid() ? m_PixelBuffer->planeSize() : Vec2i(0,0);
    }
    
    ePixelFormat Image::getPixelFormat() const
    {
        return isValid() ? m_PixelBuffer->format() : kPixelFormatInvalid;
    }
    
    intrusive_ptr<PixelBuffer> const& Image::getPixelBuffer() const
    {
        return m_PixelBuffer;
    }
    
    void Image::setPixelBuffer(intrusive_ptr<PixelBuffer> const &pb)
    {
        m_PixelBuffer = pb;
    }
    
    Image::view_t Image::getView()
    {
        if(!isValid())
            return view_t();
        
        switch(m_PixelBuffer->format())
        {
        case kPixelFormatRGB8:
            return view_t(
                gil::interleaved_view(
                    m_PixelBuffer->planeSize()[0],
                    m_PixelBuffer->planeSize()[1],
                    reinterpret_cast<PixelTraits<kPixelFormatRGB8>::pixel_t*>(m_PixelBuffer->planeData()),
                    m_PixelBuffer->planeRowBytes()
                    
                )
            );
            break;
        
        case kPixelFormatBGRA8:
            return view_t(
                gil::interleaved_view(
                    m_PixelBuffer->planeSize()[0],
                    m_PixelBuffer->planeSize()[1],
                    reinterpret_cast<PixelTraits<kPixelFormatBGRA8>::pixel_t*>(m_PixelBuffer->planeData()),
                    m_PixelBuffer->planeRowBytes()
                )
            );
            break;
        
        case kPixelFormatYUV420sp:
            return view_t(
                gil::yuv420sp_view
                <
                    PixelTraits<kPixelFormatYUV420sp>::pixel_t,
                    PixelTraits<kPixelFormatYUV420sp>::uv_swap
                >(
                    m_PixelBuffer->planeSize()[0],
                    m_PixelBuffer->planeSize()[1],
                    reinterpret_cast<uint8_t*>(m_PixelBuffer->planeData(0)), m_PixelBuffer->planeRowBytes(0),
                    reinterpret_cast<uint8_t*>(m_PixelBuffer->planeData(1)), m_PixelBuffer->planeRowBytes(1)
                )
            );
            break;
            
        default:
            return view_t();
            break;
        }
    }
    
    void Image::clear()
    {
    	m_PixelBuffer.reset();
    }

    system::error_code Image::write(fs::path const &path, eImageFormat fmt)
    {
        if(!isValid())
            return makeErrorCode(kENotApplicable);
        
        switch(fmt)
        {
        case kImageFormatJPEG:
            gil::jpeg_write_view(path.string(), gil::color_converted_view<PixelTraits<kPixelFormatRGB8>::pixel_t>(getView()));
            break;
        
        default:
            return makeErrorCode(kENotApplicable);
            break;
        }
        
        return makeErrorCode(kENoError);
    }
    
    
} //namespace base
} //namespace uquad
