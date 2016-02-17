#include <uquad/Config.h>
#include <uquad/comm/CameraServer.h>
#include <uquad/comm/CameraClient.h>
#include <uquad/base/Runloop.h>
#include <uquad/hal/DeviceManager.h>
#include <uquad/hal/Camera.h>

using namespace uquad;
int main(int argc, char *argv[])
{
    intrusive_ptr<hal::Camera> camera;
    SI(hal::DeviceManager).enumerateDevices<hal::Camera>([&camera](intrusive_ptr<hal::Camera> const &c) -> bool
	{
        camera = c;
        return false;
	});
    
    camera->open();
    

    intrusive_ptr<base::Runloop> runloop(new base::Runloop());
    comm::CameraServer cameraServer(runloop);
    
    cameraServer.setCamera(camera);
    camera->startPreview();
    camera->startRecording();

    camera->beginConfiguration();
    camera->setPreviewSize(Vec2i(640,480));
    if(!camera->getSupportedVideoSizes().empty())
    {
        camera->setVideoSize(Vec2i(640,480));
    }
    camera->endConfiguration();

    cameraServer.start();

    runloop->run();
    return 0;
}
