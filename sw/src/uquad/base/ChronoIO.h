#ifndef UQUAD_BASE_CHRONO_IO_H
#define UQUAD_BASE_CHRONO_IO_H

namespace boost { namespace archive {

    template <class Archive, typename Clock>
    inline void load(Archive &ar, boost::chrono::time_point<Clock> &tp, const unsigned int)
    {
        using namespace boost::chrono;
        typename Clock::duration::rep d;

        ar & d;
        tp = time_point<Clock>(typename Clock::duration(d));
    }

    template<class Archive, typename Clock>
    inline void save(Archive &ar, boost::chrono::time_point<Clock> const &tp, const unsigned int)
    {
        using namespace boost::chrono;
        typename Clock::duration::rep d = tp.time_since_epoch().count();
        ar & d;
    }

    template<class Archive, typename Clock>
    inline void serialize(Archive &ar, boost::chrono::time_point<Clock> &tp, const unsigned int version)
    {
        boost::serialization::split_free(ar, tp, version);
    }
    
} }

#endif //UQUAD_BASE_CHRONO_IO_H