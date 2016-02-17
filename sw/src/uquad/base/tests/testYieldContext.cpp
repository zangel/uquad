#include <boost/test/unit_test.hpp>
#include <uquad/base/Runloop.h>
#include <uquad/base/YieldContext.h>
#include <uquad/base/Error.h>

using namespace uquad;

BOOST_AUTO_TEST_CASE(YieldContext)
{
    system::error_code error;
    
    base::Runloop runloop;
    
    
    error = base::makeErrorCode(base::kENoError);
    asio::spawn(runloop.ios(), [&](asio::yield_context yield)
    {
        base::YieldContext yieldCtx(runloop.ios(), yield);
        
        runloop.ios().post([&]()
        {
            yieldCtx.done(base::makeErrorCode(base::kENotOpened));
        });
        
        error = yieldCtx.wait();
    });
    
    runloop.runAtMostOne();
    runloop.runAtMostOne();
    runloop.runAtMostOne();
    BOOST_TEST_REQUIRE(error.value() == base::kENotOpened);
    
    error = base::makeErrorCode(base::kENoError);
    asio::spawn(runloop.ios(), [&](asio::yield_context yield)
    {
        base::YieldContext yieldCtx(runloop.ios(), yield);
        
        runloop.ios().post([&]()
        {
            yieldCtx.cancel();
        });
        
        error = yieldCtx.wait();
    });
    
    runloop.runAtMostOne();
    runloop.runAtMostOne();
    runloop.runAtMostOne();
    BOOST_TEST_REQUIRE(error.value() == base::kEOperationAborted);
}
