#ifndef BOOST_GIL_EXTENSION_YUV420SP_YUV420SP_HPP
#define BOOST_GIL_EXTENSION_YUV420SP_YUV420SP_HPP

#include <boost/mpl/divides.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/vector_c.hpp>

#include <boost/gil/image.hpp>

namespace boost{ namespace gil {

typedef boost::gil::point2<std::ptrdiff_t> point_t;

template <typename Locator, bool uv_swap>
struct yuv420sp_image_deref_fn
{
    typedef gray8_view_t::locator           y_plane_locator_t;
    typedef dev2n8_view_t::locator          uv_plane_locator_t;

    typedef yuv420sp_image_deref_fn         const_t;
    typedef typename Locator::value_type    value_type;
    typedef value_type                      reference;
    typedef value_type                      const_reference;
    typedef point_t                         argument_type;
    typedef reference                       result_type;

    static const bool is_mutable = false;
    static const int u_index = uv_swap ? 1 : 0;
    static const int v_index = uv_swap ? 0 : 1;

    /// default constructor
    yuv420sp_image_deref_fn() {}

    /// constructor
    yuv420sp_image_deref_fn(const y_plane_locator_t& y_locator , const uv_plane_locator_t& uv_locator)
        : m_y_locator(y_locator)
        , m_uv_locator(uv_locator)
    {}

    /// operator()
    result_type operator()(const point_t& p) const
    {
        y_plane_locator_t y = m_y_locator.xy_at(p);
        uv_plane_locator_t uv = m_uv_locator.xy_at(p.x >> 1, p.y >> 1);
        return value_type(at_c<0>(*y), at_c<u_index>(*uv), at_c<v_index>(*uv));
    }

    /// 
    const y_plane_locator_t& y_locator() const { return m_y_locator; }
    const uv_plane_locator_t& uv_locator() const { return m_uv_locator; }

private:
    
    y_plane_locator_t m_y_locator;
    uv_plane_locator_t m_uv_locator;
};


template <typename Locator, bool uv_swap>
struct yuv420sp_image_locator
{
    typedef virtual_2d_locator
    <
        yuv420sp_image_deref_fn<Locator, uv_swap>,
        false
    > type;
};


template <typename Locator, bool uv_swap>
class yuv420sp_image_view
    : public image_view<Locator>
{
public:

    typedef Locator                                 locator_t;
    typedef typename locator_t::deref_fn_t          deref_fn_t;
    typedef typename deref_fn_t::y_plane_locator_t  y_plane_locator_t;
    typedef typename deref_fn_t::uv_plane_locator_t uv_plane_locator_t;


    typedef yuv420sp_image_view                     const_t;

    typedef image_view<y_plane_locator_t>           y_plane_view_t;
    typedef image_view<uv_plane_locator_t>          uv_plane_view_t;

    /// default constructor
    yuv420sp_image_view()
        : image_view<Locator>()
    {}

    /// constructor
    yuv420sp_image_view(const point_t& dimensions, const Locator& locator)
        : image_view<Locator>(dimensions, locator)
        , m_y_dimensions(dimensions)
        , m_uv_dimensions(dimensions.x >> 1, dimensions.y >> 1)
    {}
    
    /// copy constructor
    template <typename YUV420SPView >
    yuv420sp_image_view(const YUV420SPView& v)
        : image_view<locator_t>(v)
    {}

    const point_t& y_dimension() const { return m_y_dimensions; }
    const point_t& uv_dimension() const { return m_uv_dimensions; }

    const y_plane_locator_t& y_plane() const { return get_deref_fn().y_locator(); }
    const uv_plane_locator_t& uv_plane() const { return get_deref_fn().uv_locator(); }

    const y_plane_view_t y_plane_view() const { return y_plane_view_t(m_y_dimensions, y_plane()); }
    const uv_plane_view_t uv_plane_view() const { return uv_plane_view_t(m_uv_dimensions, uv_plane()); }

private:

    const deref_fn_t& get_deref_fn() const { return this->pixels().deref_fn(); }

private:

    template <typename L, bool> friend class yuv420sp_image_view;

    point_t m_y_dimensions;
    point_t m_uv_dimensions;
};


template <typename Pixel, bool uv_swap, typename Allocator = std::allocator<unsigned char> >
class yuv420sp_image
{
public:
    
    typedef Pixel                                       pixel_t;
    typedef typename channel_type<Pixel>::type          channel_t;
    typedef pixel<channel_t, gray_layout_t>             y_pixel_t;
    typedef pixel<channel_t, devicen_layout_t<2> >      uv_pixel_t;
    

    typedef image<y_pixel_t, false, Allocator>          y_plane_image_t;
    typedef typename y_plane_image_t::view_t            y_plane_view_t;
    typedef typename y_plane_image_t::const_view_t      y_plane_const_view_t;
    typedef typename y_plane_view_t::locator            y_plane_locator_t;
    
    typedef image<uv_pixel_t, false, Allocator>         uv_plane_image_t;
    typedef typename uv_plane_image_t::view_t           uv_plane_view_t;
    typedef typename uv_plane_image_t::const_view_t     uv_plane_const_view_t;
    typedef typename uv_plane_view_t::locator           uv_plane_locator_t;

    typedef typename view_type_from_pixel<Pixel>::type  pixel_view_t;
    typedef typename pixel_view_t::locator              pixel_locator_t;

    typedef typename yuv420sp_image_locator
    <
        pixel_locator_t, uv_swap
    >::type                                             locator_t;

    typedef typename y_plane_image_t::coord_t           x_coord_t;
    typedef typename y_plane_image_t::coord_t           y_coord_t;

    typedef yuv420sp_image_view<locator_t, uv_swap>     view_t;
    typedef typename view_t::const_t                    const_view_t;


    /// constructor
    yuv420sp_image(const x_coord_t y_width, const y_coord_t y_height)
        : m_y_plane(y_width, y_height, 0, Allocator())
        , m_uv_plane(y_width >> 1, y_height >> 1, 0, Allocator())
    {
        init(point_t(y_width, y_height));
    }

public:

    view_t m_view;

private:
    
    void init( const point_t& y_dimensions )
    {
        typedef yuv420sp_image_deref_fn<pixel_locator_t, uv_swap>   defer_fn_t;

        defer_fn_t deref_fn(view(m_y_plane).xy_at(0, 0), view(m_uv_plane).xy_at(0, 0));

        // init a virtual_2d_locator
        locator_t locator(point_t(0, 0), point_t(1, 1), deref_fn);

        m_view = view_t(m_y_plane.dimensions(), locator);
    }
    

private:

    y_plane_image_t m_y_plane;
    uv_plane_image_t m_uv_plane;
};

template <typename Pixel, bool uv_swap>
inline
const typename yuv420sp_image<Pixel, uv_swap>::view_t& view(yuv420sp_image<Pixel, uv_swap> &img)
{
    return img.m_view;
}

template< typename Pixel, bool uv_swap>
inline
const typename yuv420sp_image<Pixel, uv_swap>::const_view_t const_view(yuv420sp_image<Pixel, uv_swap> &img)
{
    return static_cast<const typename yuv420sp_image<Pixel, uv_swap>::const_view_t>(img._view);
}

template <typename Locator, bool uv_swap, typename Pixel>
void fill_pixels(const yuv420sp_image_view<Locator, uv_swap> &view, const Pixel &value)
{
    typedef typename yuv420sp_image<Pixel, uv_swap>::y_plane_view_t::value_type y_channel_t;
    typedef typename yuv420sp_image<Pixel, uv_swap>::uv_plane_view_t::value_type uv_channel_t;

    fill_pixels(view.y_plane_view(), y_channel_t(at_c<0>(value)));
    fill_pixels(view.uv_plane_view(), uv_channel_t(at_c<1>(value), at_c<2>(value)));
}

template <typename Pixel, bool uv_swap>
typename yuv420sp_image<Pixel, uv_swap>::view_t yuv420sp_view(std::size_t y_width, std::size_t y_height, uint8_t *y_base, std::ptrdiff_t y_stride, uint8_t *uv_base, std::ptrdiff_t uv_stride)
{
    typedef typename yuv420sp_image<Pixel, uv_swap>::y_plane_view_t y_plane_view_t;
    y_plane_view_t y_plane = interleaved_view(y_width, y_height, (typename y_plane_view_t::value_type*) y_base, y_stride);
    
    typedef typename yuv420sp_image<Pixel, uv_swap>::uv_plane_view_t uv_plane_view_t;
    uv_plane_view_t uv_plane = interleaved_view(y_width >> 1, y_height >> 1, (typename uv_plane_view_t::value_type*) uv_base, uv_stride);

    typedef yuv420sp_image_deref_fn<typename yuv420sp_image<Pixel, uv_swap>::pixel_locator_t, uv_swap> defer_fn_t;
    defer_fn_t deref_fn(y_plane.xy_at(0, 0), uv_plane.xy_at(0, 0));
    

    typedef typename yuv420sp_image<Pixel, uv_swap>::locator_t locator_t;
    locator_t locator(point_t(0, 0), point_t(1, 1), deref_fn);

    typedef typename yuv420sp_image<Pixel, uv_swap>::view_t view_t;
    return view_t(point_t(y_width,y_height), locator);
}

} // namespace gil
} // namespace boost

#endif // BOOST_GIL_EXTENSION_YUV420SP_YUV420SP_HPP
