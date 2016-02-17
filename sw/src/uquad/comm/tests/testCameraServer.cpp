#include <boost/test/unit_test.hpp>
#include <uquad/comm/CameraServer.h>
#include <uquad/comm/CameraClient.h>
#include <uquad/base/Runloop.h>
#include <uquad/hal/DeviceManager.h>
#include <uquad/hal/Camera.h>
#include <future>



using namespace uquad;
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(CameraServer)
{
    intrusive_ptr<base::Runloop> runloop(new base::Runloop());
    atomic<bool> run(true);
    
    thread runloopThread([&run, runloop]()
    {
        runloop->runUntil([&run]() -> bool
        {
            return run;
        });
    });
    
    intrusive_ptr<hal::Camera> camera;
    SI(hal::DeviceManager).enumerateDevices<hal::Camera>([&camera](intrusive_ptr<hal::Camera> const &c) -> bool
	{
        camera = c;
        return false;
	});
    
    BOOST_TEST_REQUIRE(camera);
    BOOST_TEST_REQUIRE(!camera->open());
    

    BOOST_TEST_REQUIRE(!camera->beginConfiguration());
    BOOST_TEST_REQUIRE(!camera->setPreviewSize(Vec2i(640,480)));
    if(!camera->getSupportedVideoSizes().empty())
    {
        BOOST_TEST_REQUIRE(!camera->setVideoSize(Vec2i(640,480)));
    }
    BOOST_TEST_REQUIRE(!camera->endConfiguration());
    
    comm::CameraServer cameraServer(runloop);
    
    BOOST_TEST_REQUIRE(!cameraServer.isStarted());
    BOOST_TEST_REQUIRE(!cameraServer.setCamera(camera));
    BOOST_TEST_REQUIRE(!camera->startPreview());
    BOOST_TEST_REQUIRE(!camera->startRecording());

    { std::promise<system::error_code> p; cameraServer.start([&p](system::error_code e) { p.set_value(e); }); p.get_future().wait(); }
    BOOST_TEST_REQUIRE(cameraServer.isStarted());
    
    comm::CameraClient cameraClient(runloop);
    BOOST_TEST_REQUIRE(!cameraClient.isConnected());
    
    { std::promise<system::error_code> p; cameraClient.connect([&p](system::error_code e) { p.set_value(e); }); p.get_future().wait(); }
    BOOST_TEST_REQUIRE(cameraClient.isConnected());
    
    /*
    this_thread::sleep_for(chrono::seconds(1));
    
    { std::promise<system::error_code> p; cameraClient.disconnect([&p](system::error_code e) { p.set_value(e); }); p.get_future().wait(); }
    BOOST_TEST_REQUIRE(!cameraClient.isConnected());
    
    { std::promise<system::error_code> p; cameraClient.connect([&p](system::error_code e) { p.set_value(e); }); p.get_future().wait(); }
    BOOST_TEST_REQUIRE(cameraClient.isConnected());
    */
    
    this_thread::sleep_for(chrono::seconds(100));
    
    { std::promise<system::error_code> p; cameraServer.stop([&p](system::error_code e) { p.set_value(e); }); p.get_future().wait(); }
    BOOST_TEST_REQUIRE(!cameraServer.isStarted());
    
    BOOST_TEST_REQUIRE(!cameraServer.setCamera(intrusive_ptr<hal::Camera>()));
    BOOST_TEST_REQUIRE(!camera->stopRecording());
    BOOST_TEST_REQUIRE(!camera->stopPreview());
    BOOST_TEST_REQUIRE(!camera->close());
    
    run = false;
    runloopThread.join();
}