#include <boost/test/unit_test.hpp>
#include <uquad/base/YieldEvent.h>
#include <uquad/base/Error.h>

using namespace uquad;

BOOST_AUTO_TEST_CASE(YieldEvent)
{
    
    
    asio::io_service ios;
    asio::io_service::work work(ios);
    
    thread runner1([&ios]
    {
        ios.run();
    });
    
    asio::steady_timer timer(ios);
    
    timer.expires_at(asio::steady_timer::time_point::max());
    
    //ios.post([&ios, &timer]()
    //{
        asio::spawn(ios, [&ios, &timer](asio::yield_context ctx)
        {
            ios.post([&timer]()
            {
                std::cout << "timer.cancel" << std::endl;
                timer.cancel();
            });
            
            system::error_code e;
            std::cout << "timer.async_wait ... " << std::endl;
            timer.async_wait(ctx[e]);
            std::cout << "timer.async_wait done: " << e << std::endl;
        });
    //});
    
    
    
    /*base::YieldEvent yieldEvt(ios);
    
    std::size_t n = 0;
    
    for(std::size_t i=0;i<1000;++i)
    {
        ios.post([&yieldEvt, &ios, &n]()
        {
            asio::spawn(ios, [&yieldEvt, &ios, &n](asio::yield_context ctx)
            {
                system::error_code e = yieldEvt.waitBlock(ctx, [&yieldEvt, &ios]() -> system::error_code
                {
                    ios.post([&yieldEvt]()
                    {
                        yieldEvt.set();
                    });
                    return base::makeErrorCode(base::kENoError);
                });
                
                if(e)
                {
                    std::cout << e << std::endl;
                }
                else
                {
                    n++;
                }
            });
        });
    }*/
    
    usleep(20000000);
    ios.stop();
    runner1.join();
    //runner2.join();
    //BOOST_TEST_REQUIRE(n == 1000);
    
}
