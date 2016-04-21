#include <boost/test/unit_test.hpp>
#include <uquad/control/BlockLibrary.h>
#include <uquad/control/PoseEstimation.h>

using namespace uquad;
namespace tt = boost::test_tools;

void testPoseEstimation(intrusive_ptr<control::PoseEstimation> const &pe)
{
	BOOST_TEST_REQUIRE(pe->isValid());
}

BOOST_AUTO_TEST_CASE(PoseEstimation)
{
	SI(control::BlockLibrary).enumerateRegistries<control::PoseEstimation>([](intrusive_ptr<control::PoseEstimation::Registry> const &registry) -> bool
	{
        std::cout << registry->name << std::endl;
		BOOST_TEST_MESSAGE("testing pose estimation " << registry->name << "...");
        intrusive_ptr<control::PoseEstimation> pe = registry->createObject<control::PoseEstimation>();
		BOOST_TEST(pe, "failed to create pose estimation: " + registry->name);
        
		if(pe) testPoseEstimation(pe);
		
        return true;
	});
}