#include <boost/test/unit_test.hpp>
#include <uquad/comm/RemoteControlServer.h>
#include <uquad/comm/RemoteControlClient.h>
#include <uquad/comm/msg/ArmMotors.h>
#include <future>


using namespace uquad;
namespace tt = boost::test_tools;


BOOST_AUTO_TEST_CASE(RemoteControl)
{
    intrusive_ptr<base::Runloop> runloop(new base::Runloop());
    
    thread runloopThread([runloop]()
    {
        runloop->run();
    });
    
    
    comm::RemoteControlClient rcClient(runloop);
    comm::RemoteControlServer rcServer(runloop);
    
    
    { std::promise< void > promise; rcClient.start([&promise](system::error_code e){ promise.set_value(); }); promise.get_future().wait(); }
    { std::promise< void > promise; rcServer.start([&promise](system::error_code e){ promise.set_value(); }); promise.get_future().wait(); }
    
    
    ::usleep(1000000);
    rcClient.sendMessage(intrusive_ptr<comm::Message>(new comm::msg::ArmMotors(false)));
    
    ::usleep(1000000);
    
    { std::promise< void > promise; rcServer.stop([&promise](system::error_code e){ promise.set_value(); }); promise.get_future().wait(); }
    { std::promise< void > promise; rcClient.stop([&promise](system::error_code e){ promise.set_value(); }); promise.get_future().wait(); }
    
    runloop->stop();
    runloopThread.join();
}
