#include <boost/test/unit_test.hpp>
#include <uquad/media/ComponentFactory.h>
#include <uquad/media/VideoDecoder.h>

using namespace uquad;
namespace tt = boost::test_tools;

/*
void test_video_decoder(intrusive_ptr<media::VideoDecoder> const &ve)
{
	BOOST_TEST(ve->isValid());
}*/

BOOST_AUTO_TEST_CASE(VideoDecoder)
{
	/*SI(media::ComponentFactory).enumerateComponents<media::VideoDecoder>([](intrusive_ptr<media::VideoDecoder::Registry> const &registry) -> bool
	{
		BOOST_TEST_MESSAGE("testing video decoder " << registry->name << "...");
		intrusive_ptr<media::VideoDecoder> ve = registry->createComponent<media::VideoDecoder>();
		BOOST_TEST(ve);
		if(ve)
			test_video_decoder(ve);
		else
			BOOST_TEST_MESSAGE("failed to create video decoder: " << registry->name << "...");
		return true;
	});*/
}
