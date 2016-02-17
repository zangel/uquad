#ifndef UQUAD_BASE_IMAGE_H
#define UQUAD_BASE_IMAGE_H

#include "Config.h"
#include "RefCounted.h"
#include "PixelFormat.h"
#include "PixelTraits.h"
#include "ImageFormat.h"
#include "PixelBuffer.h"

namespace uquad
{
namespace base
{
    class Image
        : public RefCounted
    {
    public:
        typedef gil::any_image_view
        <
            mpl::vector
            <
                PixelTraits<kPixelFormatRGB8>::view_t,
                PixelTraits<kPixelFormatBGRA8>::view_t,
                PixelTraits<kPixelFormatYUV420sp>::view_t
            >
        > view_t;
        
        Image();
        Image(intrusive_ptr<PixelBuffer> const &pb);
        ~Image();
        
        bool isValid() const;
        
        Vec2i getSize() const;
        ePixelFormat getPixelFormat() const;
        
        intrusive_ptr<PixelBuffer> const& getPixelBuffer() const;
        void setPixelBuffer(intrusive_ptr<PixelBuffer> const &pb);
        
        void clear();
        
        view_t getView();
        
        system::error_code write(fs::path const &path, eImageFormat fmt);
        
        
    private:
        intrusive_ptr<PixelBuffer> m_PixelBuffer;
    };
    
    
} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_IMAGE_H
