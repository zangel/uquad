#include <boost/test/unit_test.hpp>
#include <uquad/control/System.h>
#include <uquad/control/QuadMotorsThrustSimple.h>

using namespace uquad;
namespace tt = boost::test_tools;

#define TEST_SYSTEM_RUN_METHOD(method)          \
    done = false;                               \
    system->method([&done](system::error_code)  \
    {                                           \
        done = true;                            \
    });                                         \
    runloop->runUntil([&done]() -> bool         \
    {                                           \
        return !done;                           \
    });


BOOST_AUTO_TEST_CASE(MotorsThrust)
{
    intrusive_ptr<base::Runloop> runloop(new base::Runloop());
    intrusive_ptr<control::System> system(new control::System(runloop));
    
    intrusive_ptr<control::QuadMotorsThrustSimple> motorsThurst(new control::QuadMotorsThrustSimple());
    BOOST_TEST_REQUIRE(!system->addBlock(motorsThurst));
    
    bool done;
    
    TEST_SYSTEM_RUN_METHOD(start)
    BOOST_TEST_REQUIRE(system->isStarted());
    
    motorsThurst->requestUpdate();
    TEST_SYSTEM_RUN_METHOD(update)
    
    TEST_SYSTEM_RUN_METHOD(stop)
    BOOST_TEST_REQUIRE(!system->isStarted());
}