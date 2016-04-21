#ifndef UQUAD_CONFIG_H
#define UQUAD_CONFIG_H

#include <boost/config.hpp>
#include <boost/preprocessor.hpp>

#include <boost/chrono.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <boost/signals.hpp>
#include <boost/array.hpp>
#include <boost/format.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/accumulators/statistics/rolling_window.hpp>
#include <boost/accumulators/statistics/rolling_variance.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>

#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/thread.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/lock_traits.hpp>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/strict_lock.hpp>


#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/composite_key.hpp>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/thread/thread.hpp>
#include <boost/detail/lightweight_mutex.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/atomic.hpp>
#include <boost/dynamic_bitset.hpp>


#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <boost/iostreams/stream_buffer.hpp>

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>

#include <boost/program_options.hpp>

#include <Eigen/Eigen>
#include <Eigen/StdVector>
#include <Eigen/Geometry>

namespace uquad
{
    using namespace Eigen;
    using namespace boost;
    
    typedef Matrix<float, 2, 1> Vec2f;
	typedef Matrix<float, 3, 1> Vec3f;
	typedef Matrix<float, 4, 1> Vec4f;
    typedef Matrix<double, 2, 1> Vec2d;
	typedef Matrix<double, 3, 1> Vec3d;
	typedef Matrix<double, 4, 1> Vec4d;
	typedef Matrix<int32_t, 2, 1> Vec2i;
	typedef Matrix<int32_t, 3, 1> Vec3i;
	typedef Matrix<int32_t, 4, 1> Vec4i;
	typedef Matrix<uint32_t, 2, 1> Vec2ui;
	typedef Matrix<uint32_t, 3, 1> Vec3ui;
	typedef Matrix<uint32_t, 4, 1> Vec4ui;
	typedef Matrix<float, 2, 2> Mat2x2f;
	typedef Matrix<float, 3, 3> Mat3x3f;
	typedef Matrix<float, 4, 4> Mat4x4f;
	typedef AngleAxis<float> AngleAxisf;
    typedef Quaternion<float> Quaternionf;
    typedef Transform<float, 2, Eigen::Affine> Transform2af;
    typedef Transform<float, 3, Eigen::Affine> Transform3af;
    typedef Transform<float, 2, Eigen::Projective> Transform2pf;
    typedef Transform<float, 3, Eigen::Projective> Transform3pf;
    
    typedef boost::detail::spinlock fast_mutex;
    
    namespace mi
    {
        using boost::multi_index_container;
        using namespace boost::multi_index;

    } //namespace mi
    
    namespace po = boost::program_options;
    namespace fs = boost::filesystem;
    namespace ios = boost::iostreams;
    

} //namespace uquad

#endif //UQUAD_CONFIG_H
