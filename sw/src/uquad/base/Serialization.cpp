#include "Serialization.h"

namespace uquad
{
namespace base
{
    IArchive::IArchive(std::istream &is)
        : primitive_base_t(*is.rdbuf(),  true)
        , archive_base_t(archive::no_header)
    {
        init();
    }
    
    IArchive::IArchive(std::basic_streambuf<std::istream::char_type, std::istream::traits_type> &bsb)
        : primitive_base_t(bsb, true)
        , archive_base_t(archive::no_header)
    {
        init();
    }
    
    void IArchive::init()
    {
    }
    
    void IArchive::load(std::string &s)
    {
        uint32_t l;
        this->primitive_base_t::load(l);
        s.resize(l);
        if(0 < l)
            this->primitive_base_t::load_binary(&(*s.begin()), l);
    }
    
    void IArchive::load_override(boost::serialization::collection_size_type &t)
    {
        uint32_t x = 0;
        *this >> x;
        t = boost::serialization::collection_size_type(x);
    }
    
    
    void IArchive::load_override(boost::archive::class_name_type &t)
    {
        std::string cn;
        cn.reserve(BOOST_SERIALIZATION_MAX_KEY_SIZE);
        load_override(cn);
        std::memcpy(t, cn.data(), cn.size());
        t.t[cn.size()] = '\0';
    }
    
    OArchive::OArchive(std::ostream & os)
        : primitive_base_t(*os.rdbuf(), true)
        , archive_base_t(archive::no_header)
    {
        init();
    }

    OArchive::OArchive(std::basic_streambuf<std::ostream::char_type, std::ostream::traits_type> &bsb)
        : primitive_base_t(bsb, true)
        , archive_base_t(archive::no_header)
    {
        init();
    }
    
    void OArchive::init()
    {
    }
    
    void OArchive::save(const std::string &s)
    {
        uint32_t l = static_cast<uint32_t>(s.size());
        this->primitive_base_t::save(l);
        this->primitive_base_t::save_binary(s.data(), l);
    }
    
    void OArchive::save_override(const boost::serialization::collection_size_type &t)
    {
        uint32_t x = t;
        *this << x;
    }
    
    void OArchive::save_override(const boost::archive::class_name_type &t)
    {
        const std::string s(t);
        *this << s;
    }
    
} //namespace base
} //namespace uquad

#include <boost/archive/impl/archive_serializer_map.ipp>
#include <boost/archive/impl/basic_binary_iprimitive.ipp>
#include <boost/archive/impl/basic_binary_oprimitive.ipp>

namespace boost
{
namespace archive
{

namespace detail
{
    template class archive_serializer_map<uquad::base::IArchive>;
    template class archive_serializer_map<uquad::base::OArchive>;
}

template class basic_binary_iprimitive
<
    uquad::base::IArchive,
    std::istream::char_type, 
    std::istream::traits_type
>;

template class basic_binary_oprimitive
<
    uquad::base::OArchive,
    std::istream::char_type, 
    std::istream::traits_type
>;

} // namespace archive
} // namespace boost