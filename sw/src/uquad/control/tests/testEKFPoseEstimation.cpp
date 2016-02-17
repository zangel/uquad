#include <boost/test/unit_test.hpp>
#include <uquad/control/SystemLibrary.h>
#include <uquad/control/PoseEstimation.h>


using namespace uquad;
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(EKFPoseEstimation)
{
    intrusive_ptr<control::PoseEstimation> poseEstimation = SI(control::SystemLibrary).createPoseEstimation("ekf");

}