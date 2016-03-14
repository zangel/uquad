#include <boost/test/unit_test.hpp>
#include <uquad/control/AccelerometerCalibration.h>

using namespace uquad;
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(AccelerometerCalibration)
{
    for(std::size_t d = 0; d < control::AccelerometerCalibration::POINTS_COUNT; ++d)
    {
        BOOST_TEST_REQUIRE(control::AccelerometerCalibration::indexOfDirection(control::AccelerometerCalibration::directionOfIndex(d)) == d);
    }
    
    control::AccelerometerCalibration scal;
    scal.setStatsWindowSize(1);
    for(std::size_t d = 0; d < control::AccelerometerCalibration::POINTS_COUNT; ++d)
    {
        scal.process(control::AccelerometerCalibration::directionOfIndex(d));
    }
    
    scal.update();
}