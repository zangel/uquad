#include <boost/test/unit_test.hpp>
#include <uquad/comm/Channel.h>
#include <uquad/comm/Message.h>
#include <uquad/comm/msg/ArmMotors.h>
#include <future>


using namespace uquad;
namespace tt = boost::test_tools;

template <typename Msg>
intrusive_ptr<Msg> channelMsg(comm::Channel &cc, comm::Channel &sc, intrusive_ptr<comm::Message> msg)
{
    sc.asyncSendMessage(msg, [](intrusive_ptr<comm::Message>, system::error_code){});
    
    std::promise< intrusive_ptr<comm::Message> > promise_msg;
    cc.asyncReceiveMessage([&promise_msg](intrusive_ptr<comm::Message> msg, system::error_code e)
    {
        promise_msg.set_value(msg);
    });
    
    auto future_msg = promise_msg.get_future();
    future_msg.wait();
    return dynamic_pointer_cast<Msg>(future_msg.get());
}

void testChanneledMsg(intrusive_ptr<comm::Message> expected, intrusive_ptr<comm::Message> channeled)
{
    BOOST_TEST_REQUIRE(channeled);
    BOOST_TEST_REQUIRE(expected->time == channeled->time);
}


BOOST_AUTO_TEST_CASE(Channel)
{
    asio::io_service io_service;
    asio::io_service::work io_service_work(io_service);
    
    thread worker([&io_service]
    {
        io_service.run();
    });
    
    asio::ip::tcp::acceptor acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 0));
    asio::ip::tcp::endpoint server_endpoint = acceptor.local_endpoint();
    server_endpoint.address(asio::ip::address_v4::loopback());
    
    asio::ip::tcp::socket client_socket(io_service);
    client_socket.connect(server_endpoint);
    
    asio::ip::tcp::socket server_socket(io_service);
    acceptor.accept(server_socket);
    
    comm::Channel client_channel(io_service);
    client_channel.setSocket(std::move(client_socket));
    
    comm::Channel server_channel(io_service);
    server_channel.setSocket(std::move(server_socket));
    
    {
        intrusive_ptr<comm::msg::ArmMotors> expectedArmMotors(new comm::msg::ArmMotors(false));
        intrusive_ptr<comm::msg::ArmMotors> channeledArmMotors = channelMsg<comm::msg::ArmMotors>(client_channel, server_channel, expectedArmMotors);
        testChanneledMsg(expectedArmMotors, channeledArmMotors);
        BOOST_TEST_REQUIRE(expectedArmMotors->arm == channeledArmMotors->arm);
    }
    io_service.stop();
    worker.join();
    
}
