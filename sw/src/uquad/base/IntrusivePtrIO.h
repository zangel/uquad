#ifndef UQUAD_BASE_INTRUSIVE_PTR_IO_H
#define UQUAD_BASE_INTRUSIVE_PTR_IO_H

// define macro to let users of these compilers do this
#define UQUAD_BASE_INTRUSIVE_PTR(T)                                             \
    BOOST_CLASS_VERSION(::boost::intrusive_ptr<T>, 1)                           \
    BOOST_CLASS_TRACKING(::boost::intrusive_ptr<T>,                             \
        ::boost::serialization::track_never)


namespace boost { namespace serialization {

    template <typename Archive, typename T>
    inline void save(Archive& ar, boost::intrusive_ptr<T> const& t, unsigned int const)
    {
        T const* ptr = t.get();
        ar << ptr;
    }

    template <typename Archive, typename T>
    inline void load(Archive& ar, boost::intrusive_ptr<T>& t, const unsigned int)
    {
        T* ptr;
        ar >> ptr;
        t.reset(ptr);
    }

    template <typename Archive, typename T>
    inline void serialize(Archive& ar, boost::intrusive_ptr<T>& t, unsigned int const version)
    {
        boost::serialization::split_free(ar, t, version);
    }
}}

#endif //UQUAD_BASE_INTRUSIVE_PTR_IO_H