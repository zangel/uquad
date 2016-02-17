#include <boost/test/unit_test.hpp>
#include <uquad/media/ComponentFactory.h>
#include <uquad/media/VideoEncoder.h>

using namespace uquad;
namespace tt = boost::test_tools;

void test_video_encoder(intrusive_ptr<media::VideoEncoder> const &ve)
{
	BOOST_TEST(ve->isValid());
}

BOOST_AUTO_TEST_CASE(VideoEncoder)
{
	SI(media::ComponentFactory).enumerateComponents<media::VideoEncoder>([](intrusive_ptr<media::VideoEncoder::Registry> const &registry) -> bool
	{
		BOOST_TEST_MESSAGE("testing video encoder " << registry->name << "...");
		intrusive_ptr<media::VideoEncoder> ve = registry->createComponent<media::VideoEncoder>();
		BOOST_TEST(ve);
		if(ve)
			test_video_encoder(ve);
		else
			BOOST_TEST_MESSAGE("failed to create video encoder: " << registry->name << "...");
		return true;
	});
}
