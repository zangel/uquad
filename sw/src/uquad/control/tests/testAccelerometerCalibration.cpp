#include <boost/test/unit_test.hpp>
#include <uquad/control/AccelerometerCalibration.h>

using namespace uquad;
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(AccelerometerCalibration)
{
    for(std::size_t d = 0; d < control::AccelerometerCalibration::DIRECTION_COUNT; ++d)
    {
        BOOST_TEST_REQUIRE(control::AccelerometerCalibration::directionIndex(control::AccelerometerCalibration::direction(d)) == d);
    }
    
    control::AccelerometerCalibration scal;
    scal.setStatsWindowSize(1);
    for(std::size_t d = 0; d < control::AccelerometerCalibration::DIRECTION_COUNT; ++d)
    {
        scal.processSample(control::AccelerometerCalibration::direction(d));
    }
    
    scal.update();
}