#include <boost/test/unit_test.hpp>
#include <uquad/control/GyroscopeCalibration.h>

using namespace uquad;
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(GyroscopeCalibration)
{
    control::GyroscopeCalibration scal;
    scal.setStatsWindowSize(1);
    scal.processSample(Vec3f(0.1f, 0.1f, 0.1f));
    scal.update();
}