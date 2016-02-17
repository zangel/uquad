#include <boost/test/unit_test.hpp>
#include <uquad/media/ComponentFactory.h>
#include <uquad/media/VideoEncoder.h>
#include <uquad/media/VideoDecoder.h>
#include <uquad/media/Writer.h>
#include <uquad/media/Reader.h>
#include <uquad/hal/DeviceManager.h>
#include <uquad/hal/Camera.h>
#include <uquad/media/Capture.h>
#include <uquad/media/Graph.h>
#include <uquad/base/Runloop.h>
#include <uquad/base/IOFileStream.h>
#include <uquad/base/Application.h>


#define USE_DECODER 0


using namespace uquad;
namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(GraphEncode)
{
    intrusive_ptr<base::Runloop> runloop(new base::Runloop());
    
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
    //BOOST_TEST_REQUIRE(!camera->setPreviewFPS(25.0f));
    BOOST_TEST_REQUIRE(!camera->endConfiguration());

    intrusive_ptr<media::Capture> capture(new media::Capture());
    capture->setCamera(camera);
    BOOST_TEST_REQUIRE(capture->isValid());
    
    intrusive_ptr<media::VideoEncoder> videoEncoder = SI(media::ComponentFactory).createVideoEncoder(media::kVideoCodecTypeH264);
    BOOST_TEST_REQUIRE(videoEncoder);
    BOOST_TEST_REQUIRE(videoEncoder->isValid());
    
    intrusive_ptr<media::Writer> writer = SI(media::ComponentFactory).createWriter(media::kContainerFormatRAW);
    BOOST_TEST_REQUIRE(writer);
    BOOST_TEST_REQUIRE(writer->isValid());
    intrusive_ptr<base::IOStream> writerStream(new base::IOFileStream((base::Application::dataDirectory()/"test.h264").string()));
    BOOST_TEST_REQUIRE(!writer->setStream(writerStream));
    
    intrusive_ptr<media::Graph> graph(new media::Graph(runloop));
    
    BOOST_TEST_REQUIRE(!graph->addComponent(capture));
    BOOST_TEST_REQUIRE(!graph->addComponent(videoEncoder));
    BOOST_TEST_REQUIRE(!graph->addComponent(writer));
    
    BOOST_TEST_REQUIRE(!graph->connect(capture, videoEncoder));
    BOOST_TEST_REQUIRE(!graph->connect(videoEncoder, writer));
    
    bool done;
    
    done = false; graph->start([&done](system::error_code){ done = true; }); runloop->runUntil([&done]() -> bool { return !done; } );
    BOOST_TEST_REQUIRE(graph->isStarted());
    
#if 0
    BOOST_TEST_REQUIRE(!camera->startPreview());
    BOOST_TEST_REQUIRE(!camera->startRecording());
#endif

    done = false; graph->resume([&done](system::error_code){ done = true; }); runloop->runUntil([&done]() -> bool { return !done; } );
    BOOST_TEST_REQUIRE(graph->isRunning());

    BOOST_TEST_REQUIRE(!runloop->runFor(chrono::seconds(1)));

#if 1
    done = false; graph->stop([&done](system::error_code){ done = true; }); runloop->runUntil([&done]() -> bool { return !done; } );
    BOOST_TEST_REQUIRE(!graph->isStarted());

    done = false; graph->start([&done](system::error_code){ done = true; }); runloop->runUntil([&done]() -> bool { return !done; } );
    BOOST_TEST_REQUIRE(graph->isStarted());
    
    done = false; graph->resume([&done](system::error_code){ done = true; }); runloop->runUntil([&done]() -> bool { return !done; } );
    BOOST_TEST_REQUIRE(graph->isRunning());
    
    BOOST_TEST_REQUIRE(!runloop->runFor(chrono::seconds(1)));
#endif
    
#if 0
    BOOST_TEST_REQUIRE(!camera->stopRecording());
    BOOST_TEST_REQUIRE(!camera->stopPreview());
#endif
    
    done = false; graph->stop([&done](system::error_code){ done = true; }); runloop->runUntil([&done]() -> bool { return !done; } );
    BOOST_TEST_REQUIRE(!graph->isStarted());
    
    BOOST_TEST_REQUIRE(!graph->disconnect(videoEncoder, writer));
    BOOST_TEST_REQUIRE(!graph->disconnect(capture, videoEncoder));
    
    BOOST_TEST_REQUIRE(!graph->removeComponent(writer));
    BOOST_TEST_REQUIRE(!graph->removeComponent(videoEncoder));
    BOOST_TEST_REQUIRE(!graph->removeComponent(capture));
    
    BOOST_TEST_REQUIRE(!camera->close());
}

BOOST_AUTO_TEST_CASE(GraphDecode)
{
    intrusive_ptr<base::Runloop> runloop(new base::Runloop());
    
    intrusive_ptr<media::Reader> reader = SI(media::ComponentFactory).createReader(media::kContainerFormatRAW);
    BOOST_TEST_REQUIRE(reader);
    BOOST_TEST_REQUIRE(reader->isValid());
    intrusive_ptr<base::IOStream> readerStream(new base::IOFileStream((base::Application::dataDirectory()/"test.h264").string()));
    BOOST_TEST_REQUIRE(!reader->setStream(readerStream));
    
    
    intrusive_ptr<media::VideoDecoder> videoDecoder = SI(media::ComponentFactory).createVideoDecoder(media::kVideoCodecTypeH264);
    BOOST_TEST_REQUIRE(videoDecoder);
    BOOST_TEST_REQUIRE(videoDecoder->isValid());
    
    
    intrusive_ptr<media::Graph> graph(new media::Graph(runloop));
    
    BOOST_TEST_REQUIRE(!graph->addComponent(reader));
    BOOST_TEST_REQUIRE(!graph->addComponent(videoDecoder));
    
    BOOST_TEST_REQUIRE(!graph->connect(reader, videoDecoder));
    
    bool done;
    
    done = false; graph->start([&done](system::error_code){ done = true; }); runloop->runUntil([&done]() -> bool { return !done; } );
    BOOST_TEST_REQUIRE(graph->isStarted());
    
    done = false; graph->resume([&done](system::error_code){ done = true; }); runloop->runUntil([&done]() -> bool { return !done; } );
    BOOST_TEST_REQUIRE(graph->isRunning());
    
    BOOST_TEST_REQUIRE(!runloop->runFor(chrono::seconds(30)));
    
    done = false; graph->stop([&done](system::error_code){ done = true; }); runloop->runUntil([&done]() -> bool { return !done; } );
    BOOST_TEST_REQUIRE(!graph->isStarted());
    
    BOOST_TEST_REQUIRE(!graph->disconnect(reader, videoDecoder));
    
    BOOST_TEST_REQUIRE(!graph->removeComponent(videoDecoder));
    BOOST_TEST_REQUIRE(!graph->removeComponent(reader));
}
