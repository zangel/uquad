#ifndef UQUAD_BASE_SERIALIZATION_H
#define UQUAD_BASE_SERIALIZATION_H

#include "Config.h"
#include <boost/serialization/string.hpp>

namespace uquad
{
namespace base
{
    class IArchive
        : public boost::archive::basic_binary_iprimitive
            <
                IArchive,
                std::istream::char_type,
                std::istream::traits_type
            >
        , public boost::archive::detail::common_iarchive
            <
                IArchive
            >
    {
        typedef boost::archive::basic_binary_iprimitive
        <
            IArchive,
            std::istream::char_type,
            std::istream::traits_type
        > primitive_base_t;
        
        typedef boost::archive::detail::common_iarchive
        <
            IArchive
        > archive_base_t;
        
#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
    public:
#else
        friend archive_base_t;
        friend primitive_base_t; // since with override load below
        friend class boost::archive::detail::interface_iarchive<IArchive>;
        friend class boost::archive::load_access;
    protected:
#endif
        void init();
        
        //load
        template <class T> void load(T &t) { this->primitive_base_t::load(t); }
        void load(std::string &s);
        
        
        //load_override
        template<class T> void load_override(T &t) { this->archive_base_t::load_override(t); }
        
        void load_override(boost::serialization::collection_size_type &t);
        void load_override(boost::archive::class_name_type &t);
        
    public:
        IArchive(std::istream &is);
        IArchive(std::basic_streambuf<std::istream::char_type, std::istream::traits_type> &bsb);
    };
    
    class OArchive
        : public boost::archive::basic_binary_oprimitive
            <
                OArchive,
                std::ostream::char_type,
                std::ostream::traits_type
            >
        , public boost::archive::detail::common_oarchive
            <
                OArchive
            >
    {
        typedef boost::archive::basic_binary_oprimitive
        <
            OArchive,
            std::ostream::char_type,
            std::ostream::traits_type
        > primitive_base_t;
        
        typedef boost::archive::detail::common_oarchive
        <
            OArchive
        > archive_base_t;
        
#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
    public:
#else
        friend archive_base_t;
        friend primitive_base_t; // since with override save below
        friend class boost::archive::detail::interface_oarchive<OArchive>;
        friend class boost::archive::save_access;
    protected:
#endif
        void init();
        
        //save
        template <class T> void save(const T &t) { this->primitive_base_t::save(t); }
        void save(const std::string &s);
        
        //save_override
        template<class T> void save_override(const T &t) { this->archive_base_t::save_override(t); }
        void save_override(const boost::serialization::collection_size_type &t);
        void save_override(const boost::archive::class_name_type &t);
        
    public:
        OArchive(std::ostream &os);
        OArchive(std::basic_streambuf<std::ostream::char_type, std::ostream::traits_type> &bsb);
    };
    
} //namespace base
} //namespace uquad

BOOST_SERIALIZATION_REGISTER_ARCHIVE(uquad::base::IArchive)
BOOST_SERIALIZATION_REGISTER_ARCHIVE(uquad::base::OArchive)
BOOST_SERIALIZATION_USE_ARRAY_OPTIMIZATION(uquad::base::IArchive)
BOOST_SERIALIZATION_USE_ARRAY_OPTIMIZATION(uquad::base::OArchive)


#define UQUAD_BASE_SERIALIZATION_EXPORT(clazz)                                      \
    BOOST_CLASS_IMPLEMENTATION(clazz, boost::serialization::object_serializable)    \
    BOOST_CLASS_TRACKING(clazz, boost::serialization::track_never)                  \
    BOOST_CLASS_EXPORT_KEY(clazz)                                                   \

#define UQUAD_BASE_SERIALIZATION_EXPORT_IMPLEMENT(clazz)                            \
    template void clazz::serialize<uquad::base::IArchive &>(                        \
        uquad::base::IArchive &,                                                    \
        const unsigned int                                                          \
    );                                                                              \
    template void clazz::serialize<uquad::base::OArchive &>(                        \
        uquad::base::OArchive &,                                                    \
        const unsigned int                                                          \
    );                                                                              \
    BOOST_CLASS_EXPORT_IMPLEMENT(clazz)


#endif //UQUAD_BASE_SERIALIZATION_H
