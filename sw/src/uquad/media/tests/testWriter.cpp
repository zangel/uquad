#include <boost/test/unit_test.hpp>
#include <uquad/media/ComponentFactory.h>
#include <uquad/media/Writer.h>

using namespace uquad;
namespace tt = boost::test_tools;

void test_writer(intrusive_ptr<media::Writer> const &w)
{
	BOOST_TEST(w->isValid());
}

BOOST_AUTO_TEST_CASE(Writer)
{
	SI(media::ComponentFactory).enumerateComponents<media::Writer>([](intrusive_ptr<media::Writer::Registry> const &registry) -> bool
	{
		BOOST_TEST_MESSAGE("testing writer " << registry->name << "...");
		intrusive_ptr<media::Writer> w = registry->createComponent<media::Writer>();
		BOOST_TEST(w);
		if(w)
			test_writer(w);
		else
			BOOST_TEST_MESSAGE("failed to create writer: " << registry->name << "...");
		return true;
	});
}
