#include <boost/test/unit_test.hpp>
#include <uquad/control/System.h>
#include <uquad/control/PositionControlSimple.h>

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


BOOST_AUTO_TEST_CASE(PositionControl)
{
    intrusive_ptr<base::Runloop> runloop(new base::Runloop());
    intrusive_ptr<control::System> system(new control::System(runloop));
    
    intrusive_ptr<control::PositionControlSimple> positionControlSimple(new control::PositionControlSimple());
    BOOST_TEST_REQUIRE(!system->addBlock(positionControlSimple));
    
    bool done;
    
    TEST_SYSTEM_RUN_METHOD(start)
    BOOST_TEST_REQUIRE(system->isStarted());
    
    positionControlSimple->requestUpdate();
    TEST_SYSTEM_RUN_METHOD(update)
    
    TEST_SYSTEM_RUN_METHOD(stop)
    BOOST_TEST_REQUIRE(!system->isStarted());
}



