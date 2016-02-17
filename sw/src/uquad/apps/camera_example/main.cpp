#include <uquad/Config.h>
#include <uquad/hal/Camera.h>

using namespace uquad;

void printSize(Vec2i const &sz)
{
	std::cout << "  " << sz(0) << "x" << sz(1) << std::endl;
}

void printSizeArray(std::vector<Vec2i> const &sz)
{
	std::for_each(sz.begin(), sz.end(), [](Vec2i const &s)
	{
		std::cout << "  " << s(0) << "x" << s(1) << std::endl;
	});
}

void printCameraInfo(hal::Camera &camera)
{
	std::cout << " supported preview sizes: " << std::endl;
	printSizeArray(camera.getSupportedPreviewSizes());

	std::cout << " supported picture sizes: " << std::endl;
	printSizeArray(camera.getSupportedPictureSizes());

	std::cout << " preview frame rate: " << std::endl;
	std::cout << "  " << camera.getPreviewFPS() << std::endl;

	std::cout << " preview size: " << std::endl;
	printSize(camera.getPreviewSize());

	std::cout << " picture size: " << std::endl;
	printSize(camera.getPictureSize());
}

int main(int argc, char *argv[])
{
	uint32_t num_cameras = hal::Camera::numCameras();

	std::cout << "num cameras: " << num_cameras << std::endl;

	for(uint32_t icam = 0; icam < num_cameras; ++icam)
	{
		hal::Camera camera(icam);
		if(camera.open())
		{
			std::cout << "failed to open camera[" << icam << "]!" << std::endl;
			continue;
		}
		std::cout << "Camera[" << icam << "] info:" << std::endl;
		printCameraInfo(camera);
		if(camera.startPreview())
		{
			std::cout << "failed to start preview camera[" << icam << "]!" << std::endl;
			camera.stopPreview();
		}

		camera.close();
	}
	return 0;
}
