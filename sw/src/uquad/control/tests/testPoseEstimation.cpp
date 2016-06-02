#include <boost/test/unit_test.hpp>
#include <uquad/control/BlockLibrary.h>
#include <uquad/control/blocks/PoseEstimation.h>

using namespace uquad;
namespace tt = boost::test_tools;

void testPoseEstimation(intrusive_ptr<control::blocks::PoseEstimation> const &pe)
{
	BOOST_TEST_REQUIRE(pe->isValid());
}

BOOST_AUTO_TEST_CASE(PoseEstimation)
{
	SI(control::BlockLibrary).enumerateRegistries<control::blocks::PoseEstimation>([](intrusive_ptr<control::blocks::PoseEstimation::Registry> const &registry) -> bool
	{
        std::cout << registry->name << std::endl;
		BOOST_TEST_MESSAGE("testing pose estimation block" << registry->name << "...");
        intrusive_ptr<control::blocks::PoseEstimation> pe = registry->createObject<control::blocks::PoseEstimation>();
		BOOST_TEST(pe, "failed to create pose estimation block: " + registry->name);
        
		if(pe) testPoseEstimation(pe);
		
        return true;
	});
}
