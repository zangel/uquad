#include <boost/test/unit_test.hpp>
#include <uquad/hal/DeviceManager.h>
#include <uquad/hal/Camera.h>
#include <uquad/utility/Singleton.h>
#include <uquad/utility/Constant.h>
#include <uquad/base/Image.h>
#include <boost/foreach.hpp>

using namespace uquad;
namespace tt = boost::test_tools;

struct CameraDelegateHelper
    : public hal::CameraDelegate
{
    void onCameraPreviewFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer)
    {
        preview_frame_signal(buffer);
    }
    
    void onCameraVideoFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer)
    {
        video_frame_signal(buffer);
    }
    
    uquad::signal<void (intrusive_ptr<base::PixelSampleBuffer> const &)> preview_frame_signal;
    uquad::signal<void (intrusive_ptr<base::PixelSampleBuffer> const &)> video_frame_signal;
};


void test_camera_device(intrusive_ptr<hal::Camera> const &camera)
{
	BOOST_TEST_REQUIRE(!camera->isOpen());
	BOOST_TEST_REQUIRE(!camera->isPreviewStarted());
	BOOST_TEST_REQUIRE(camera->getSupportedPreviewFormats().empty());
	BOOST_TEST_REQUIRE(camera->getSupportedPreviewSizes().empty());
	BOOST_TEST_REQUIRE(camera->getSupportedPreviewFPS().empty());
	BOOST_TEST_REQUIRE(camera->getPreviewFormat() == base::kPixelFormatInvalid);
	BOOST_TEST_REQUIRE(camera->getPreviewSize() == utility::Constant<Vec2i>::value());
	BOOST_TEST_REQUIRE(camera->getPreviewFPS() == 0.0f, tt::tolerance(1.0e-3f));
	BOOST_TEST_REQUIRE(camera->getSupportedPictureFormats().empty());
	BOOST_TEST_REQUIRE(camera->getSupportedPictureSizes().empty());
	BOOST_TEST_REQUIRE(camera->getPictureFormat() == base::kImageFormatInvalid);
	BOOST_TEST_REQUIRE(camera->getPictureSize() == utility::Constant<Vec2i>::value());
	BOOST_TEST_REQUIRE(!camera->open());
	BOOST_TEST_REQUIRE(camera->isOpen());
    
    BOOST_TEST_REQUIRE(!camera->getSupportedPreviewFormats().empty());
	BOOST_TEST_REQUIRE(!camera->getSupportedPreviewSizes().empty());
	BOOST_TEST_REQUIRE(!camera->getSupportedPreviewFPS().empty());
	BOOST_TEST_REQUIRE(camera->getPreviewFormat() != base::kPixelFormatInvalid);
	BOOST_TEST_REQUIRE(camera->getPreviewSize() != utility::Constant<Vec2i>::value());
	BOOST_TEST_REQUIRE(camera->getPreviewFPS() != 0.0f, tt::tolerance(1.0e-3f));
	BOOST_TEST_REQUIRE(!camera->getSupportedPictureFormats().empty());
	BOOST_TEST_REQUIRE(!camera->getSupportedPictureSizes().empty());
	BOOST_TEST_REQUIRE(camera->getPictureFormat() != base::kImageFormatInvalid);
	BOOST_TEST_REQUIRE(camera->getPictureSize() != utility::Constant<Vec2i>::value());
    
    /*
    BOOST_TEST_REQUIRE(!camera->startPreview());
    BOOST_TEST_REQUIRE(camera->isPreviewStarted());
    BOOST_TEST_REQUIRE(!camera->stopPreview());
    BOOST_TEST_REQUIRE(!camera->isPreviewStarted());
    */

    #if 1
    CameraDelegateHelper dh;
    {
        hal::Camera::camera_delegate_t::ScopedSubscription subscription = camera->cameraDelegate() += &dh;

        bool first_frame = true;
        uquad::signals::connection sc = dh.video_frame_signal.connect([&](intrusive_ptr<base::PixelSampleBuffer> const &buffer) -> void
        {
            /*if(first_frame)
            {
                first_frame=false;
                if(!buffer->lock())
                {
                    std::ostringstream filename;
                    filename << camera->name() << "_video.jpg";
                    base::Image img(buffer);
                    img.write(filename.str(), base::kImageFormatJPEG);
                    buffer->unlock();
                }
                else
                {
                    std::cout << "failed to lock buffer" << std::endl;
                }
            }*/
        });

        camera->beginConfiguration();
        camera->setPreviewSize(Vec2i(320,240));
        camera->setVideoSize(Vec2i(640,480));
        camera->endConfiguration();
        camera->startPreview();
        //::usleep(2000000);
        camera->startRecording();
        ::usleep(10000000);
        camera->stopRecording();
        camera->stopPreview();
    }

    #else
    CameraDelegateHelper dh;
    {
        hal::Camera::camera_delegate_t::ScopedSubscription subscription = camera->cameraDelegate() += &dh;
        
        BOOST_FOREACH(base::ePixelFormat fmt, camera->getSupportedPreviewFormats())
        {
            if(fmt == base::kPixelFormatBGRA8 || fmt == base::kPixelFormatRGB8 || fmt == base::kPixelFormatYUV420sp)
            {
                BOOST_TEST_REQUIRE(!camera->beginConfiguration());
                BOOST_TEST_REQUIRE(!camera->setPreviewFormat(fmt));
                BOOST_TEST_REQUIRE(!camera->endConfiguration());
                BOOST_TEST_REQUIRE(camera->getPreviewFormat() == fmt);


                BOOST_FOREACH(Vec2i sz, camera->getSupportedPreviewSizes())
                {
                    BOOST_TEST_REQUIRE(!camera->beginConfiguration());
                    BOOST_TEST_REQUIRE(!camera->setPreviewSize(sz));
                    BOOST_TEST_REQUIRE(!camera->endConfiguration());
                    BOOST_TEST_REQUIRE(camera->getPreviewSize() == sz);

                    BOOST_FOREACH(float fps, camera->getSupportedPreviewFPS())
                    {
                        BOOST_TEST_REQUIRE(!camera->beginConfiguration());
                        BOOST_TEST_REQUIRE(!camera->setPreviewFPS(fps));
                        BOOST_TEST_REQUIRE(!camera->endConfiguration());

                        BOOST_TEST(camera->getPreviewFPS() == fps, tt::tolerance(1.0e-2f));

                        chrono::high_resolution_clock::time_point preview_start, preview_end;

                        uint32_t num_frames = 0;
                        bool first_frame = true;
                        uquad::signals::connection sc = dh.preview_frame_signal.connect([&](intrusive_ptr<base::PixelSampleBuffer> const &buffer) -> void
                        {
                            if(first_frame)
                            {
                                first_frame=false;
                                preview_start = chrono::high_resolution_clock::now();
                                if(!buffer->lock())
                                {
                                    std::ostringstream filename;
                                    filename << camera->name() << "_" << int(fmt) << "_" << sz(0) << "x" << sz(1) << "_" << fps << "_preview.jpg";
                                    base::Image img(buffer);
                                    img.write(filename.str(), base::kImageFormatJPEG);
                                    buffer->unlock();
                                }
                            }
                            else
                            {
                                preview_end = chrono::high_resolution_clock::now();
                                num_frames++;
                            }
                        });
                        camera->startPreview();
                        ::usleep(2000000);
                        camera->stopPreview();
                        BOOST_TEST_REQUIRE(num_frames > 0);

                        float estimated_preview_fps = 1000.0f*
                            (float(num_frames)/float(chrono::duration_cast<chrono::milliseconds>(preview_end - preview_start).count()));

                        BOOST_TEST(fps == estimated_preview_fps, tt::tolerance(1.0e-2f));
                        sc.disconnect();
                    }

                }
            }
        }
    }
    #endif

	BOOST_TEST_REQUIRE(!camera->close());
	BOOST_TEST_REQUIRE(camera->getSupportedPreviewFormats().empty());
	BOOST_TEST_REQUIRE(camera->getSupportedPreviewSizes().empty());
	BOOST_TEST_REQUIRE(camera->getSupportedPreviewFPS().empty());
	BOOST_TEST_REQUIRE(camera->getPreviewFormat() == base::kPixelFormatInvalid);
	BOOST_TEST_REQUIRE(camera->getPreviewSize() == utility::Constant<Vec2i>::value());
	BOOST_TEST_REQUIRE(camera->getPreviewFPS() == 0.0f, tt::tolerance(1.0e-3f));
	BOOST_TEST_REQUIRE(camera->getSupportedPictureFormats().empty());
	BOOST_TEST_REQUIRE(camera->getSupportedPictureSizes().empty());
	BOOST_TEST_REQUIRE(camera->getPictureFormat() == base::kImageFormatInvalid);
	BOOST_TEST_REQUIRE(camera->getPictureSize() == utility::Constant<Vec2i>::value());

	BOOST_TEST_REQUIRE(!camera->isOpen());
}

BOOST_AUTO_TEST_CASE(Camera)
{
    
	SI(hal::DeviceManager).enumerateDevices<hal::Camera>([](intrusive_ptr<hal::Camera> const &camera) -> bool
	{
        BOOST_TEST_MESSAGE("testing " << camera->name() << " camera ...");
		test_camera_device(camera);
        BOOST_TEST_MESSAGE("done.");
		return true;
	});
}
