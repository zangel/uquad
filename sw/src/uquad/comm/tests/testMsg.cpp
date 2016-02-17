#include <boost/test/unit_test.hpp>
#include <uquad/comm/Message.h>
#include <uquad/comm/msg/ArmMotors.h>


using namespace uquad;
namespace tt = boost::test_tools;


template <typename Msg>
intrusive_ptr<Msg> serializedMsg(intrusive_ptr<comm::Message> msg)
{
    std::stringstream ss;
    archive::binary_oarchive oa(ss, archive::no_header);
    oa << msg;
    
    std::cout << "size of " << typeid(Msg).name() << " :" << ss.str().size() << std::endl;
    std::cout << "archive :" << ss.str() << std::endl;
    intrusive_ptr<comm::Message> ret;
    
    archive::binary_iarchive ia(ss, archive::no_header);
    ia >> ret;
    return dynamic_pointer_cast<Msg>(ret);
}

void testMsg(intrusive_ptr<comm::Message> expected, intrusive_ptr<comm::Message> serialized)
{
    BOOST_TEST_REQUIRE(serialized);
    BOOST_TEST_REQUIRE(expected->time == serialized->time);
}

BOOST_AUTO_TEST_CASE(Msg)
{
    {
        intrusive_ptr<comm::msg::ArmMotors> expectedArmMotors(new comm::msg::ArmMotors(false));
        intrusive_ptr<comm::msg::ArmMotors> serializedArmMotors = serializedMsg<comm::msg::ArmMotors>(expectedArmMotors);
        testMsg(expectedArmMotors, serializedArmMotors);
        BOOST_TEST_REQUIRE(expectedArmMotors->arm == serializedArmMotors->arm);
    }
}
