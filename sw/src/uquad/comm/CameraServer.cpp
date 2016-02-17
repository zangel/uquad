#include "CameraServer.h"
#include "../media/ComponentFactory.h"
#include "../base/Error.h"
#include "../base/YieldEvent.h"
#include "Log.h"
#include <future>

namespace uquad
{
namespace comm
{
    CameraServer::CameraServer(intrusive_ptr<base::Runloop> const &rl)
        : m_State(kStateIdle)
        , m_Runloop(rl)
        , m_IOSStrand(m_Runloop->ios())
        , m_Resolver(rl->ios())
        , m_Acceptor(rl->ios())
        , m_Socket(rl->ios())
        , m_ListenAddress(asio::ip::address::from_string("0.0.0.0"))
        , m_ListenPort(35624)
        , m_Capture()
        , m_VideoEncoder(SI(media::ComponentFactory).createVideoEncoder(media::kVideoCodecTypeH264))
        , m_RawWriter()
        , m_Graph(rl)
        , m_SocketStream()
    {
        intrusive_ptr_add_ref(&m_Capture);
        intrusive_ptr_add_ref(&m_RawWriter);
        intrusive_ptr_add_ref(&m_Graph);
        intrusive_ptr_add_ref(&m_SocketStream);
        
        m_SocketStream.ioAsyncStreamDelegate() += this;
        
    }
    
    CameraServer::~CameraServer()
    {
        if(isStarted())
        {
            std::promise<system::error_code> se;
            stop([this, &se](system::error_code e) { se.set_value(e); });
            auto sefu = se.get_future();
            
            if(m_IOSStrand.running_in_this_thread())
            {
                m_Runloop->runUntil([this, &sefu]() -> bool
                {
                    return sefu.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
                });
            }
            else
            {
                sefu.wait();
            }
        }
        m_SocketStream.ioAsyncStreamDelegate() -= this;
    }
    
    system::error_code CameraServer::setListenAddress(asio::ip::address const &addr)
    {
        if(isStarted())
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::setListenAddress: cannot change address while is started!";
            return base::makeErrorCode(base::kEInvalidState);
        }
        
        m_ListenAddress = addr;
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code CameraServer::setListenPort(int port)
    {
        if(isStarted())
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::setListenAddress: cannot change port while is started!";
            return base::makeErrorCode(base::kEInvalidState);
        }
        
        m_ListenPort = port;
        return base::makeErrorCode(base::kENoError);
    }
    
    intrusive_ptr<hal::Camera> const& CameraServer::getCamera() const
    {
        return m_Capture.getCamera();
    }
    
    system::error_code CameraServer::setCamera(intrusive_ptr<hal::Camera> const &camera)
    {
        return m_Capture.setCamera(camera);
    }
    
    void CameraServer::start()
    {
        start([](system::error_code){});
    }
    
    void CameraServer::start(handler_t handler)
    {
        asio::spawn(m_IOSStrand, bind(&CameraServer::doStart, this, _1, std::move(handler)));
    }
    
    bool CameraServer::isStarted() const
    {
        return m_State >= kStateStarted;
    }
    
    bool CameraServer::isConnected() const
    {
        return m_State == kStateConnected;
    }
    
    void CameraServer::stop()
    {
        stop([](system::error_code){});
    }
    
    void CameraServer::stop(handler_t handler)
    {
        asio::spawn(m_IOSStrand, bind(&CameraServer::doStop, this, _1, std::move(handler)));
    }
    
    void CameraServer::doStart(asio::yield_context yctx, handler_t handler)
    {
        if(isStarted())
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::doStart: already started!";
            handler(base::makeErrorCode(base::kEAlreadyStarted));
            return;
        }
        
        if(!makeGraph())
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::doStart: failed to make the graph!";
            handler(base::makeErrorCode(base::kENotStarted));
            return;
        }
        
        if(!startGraph(yctx))
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::doStart: failed to start the graph!";
            cleanGraph();
            handler(base::makeErrorCode(base::kENotStarted));
            return;
        }
        
        
        system::error_code err;
        asio::ip::tcp::resolver::query listenQuery(m_ListenAddress.to_string(), boost::lexical_cast<std::string>(m_ListenPort));
        asio::ip::tcp::resolver::iterator itEndpoint = m_Resolver.resolve(listenQuery, err);
        
        if(err || itEndpoint == asio::ip::tcp::resolver::iterator())
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::doStart: failed to resolve host:port, e=" << err;
            stopGraph(yctx);
            cleanGraph();
            handler(base::makeErrorCode(base::kENotStarted));
            return;
        }
            
        
        asio::ip::tcp::endpoint listenEndpoint = *itEndpoint;
        
        
        m_Acceptor.open(listenEndpoint.protocol(), err);
        if(err)
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::doStart: m_Acceptor.open failed, e=" << err;
            stopGraph(yctx);
            cleanGraph();
            handler(base::makeErrorCode(base::kENotStarted));
            return;
        }
        
        m_Acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true), err);
        if(err)
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::doStart: m_Acceptor.set_option failed, e=" << err;
            m_Acceptor.close(err);
            stopGraph(yctx);
            cleanGraph();
            handler(base::makeErrorCode(base::kENotStarted));
            return;
        }
        
        m_Acceptor.bind(listenEndpoint, err);
        if(err)
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::doStart: m_Acceptor.bind failed, e=" << err;
            m_Acceptor.close(err);
            stopGraph(yctx);
            cleanGraph();
            handler(base::makeErrorCode(base::kENotStarted));
            return;
        }
        
        m_Acceptor.listen(boost::asio::socket_base::max_connections, err);
        if(err)
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::doStart: m_Acceptor.listen failed, e=" << err;
            m_Acceptor.close(err);
            stopGraph(yctx);
            cleanGraph();
            handler(base::makeErrorCode(base::kENotStarted));
            return;
        }
        
        UQUAD_COMM_LOG(Debug) << "CameraServer::doStart: started successfully!";
        m_State = kStateStarted;
        startAccept();
        handler(base::makeErrorCode(base::kENoError));
    }
    
    void CameraServer::doStop(asio::yield_context yctx, handler_t handler)
    {
        if(!isStarted())
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::doStop: not started!";
            handler(base::makeErrorCode(base::kENotStarted));
            return;
        }
        
        if(isConnected())
        {
            base::YieldEvent pauseEvent(m_Runloop->ios());
            system::error_code pe = pauseEvent.waitBlock(yctx, [this, &pauseEvent]() -> system::error_code
            {
                m_Graph.pause([&pauseEvent](system::error_code e)
                {
                    pauseEvent.set(e);
                });
                return base::makeErrorCode(base::kENoError);
            });

            if(pe)
            {
                UQUAD_COMM_LOG(Error) << "CameraServer::doStop: failed to pause the graph: " << pe;
            }
            m_SocketStream.close();
            m_State = kStateStarted;
        }
        
        stopGraph(yctx);
        cleanGraph();
        UQUAD_COMM_LOG(Debug) << "CameraServer::doStop: stopped!";
        m_State = kStateIdle;
        handler(base::makeErrorCode(base::kENoError));
        
        system::error_code err;
        m_Acceptor.cancel(err);
        m_Acceptor.close(err);
        
        m_Socket.cancel(err);
        m_Socket.close(err);
        m_Socket.shutdown(asio::ip::tcp::socket::shutdown_both, err);
    }
    
    void CameraServer::onIOAsyncSocketStreamError(system::error_code const &e)
    {
        UQUAD_COMM_LOG(Debug) << "CameraServer::onIOAsyncSocketStreamError:" << e;
        m_Graph.pause([this](system::error_code pe)
        {
            if(pe)
            {
                UQUAD_COMM_LOG(Error) << "CameraServer::onIOAsyncSocketStreamDisconnected: failed to pause the graph:" << pe;
            }
            m_SocketStream.close();
            m_State = kStateStarted;
            startAccept();
            UQUAD_COMM_LOG(Debug) << "CameraServer::onIOAsyncSocketStreamDisconnected: disconnected";
        });
    }
    
    
    bool CameraServer::makeGraph()
    {
        intrusive_ptr<hal::Camera> camera = getCamera();
        if(!camera || !camera->isOpen() || !camera->isRecordingStarted())
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::makeGraph: camera not set or open failed";
            return false;
        }
        
        if(!m_VideoEncoder)
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::makeGraph: video encoder not valid";
            return false;
        }
        
        m_RawWriter.setStream(intrusive_ptr<base::IOStream>(&m_SocketStream));
        
        m_Graph.addComponent(intrusive_ptr<media::Component>(&m_Capture));
        m_Graph.addComponent(m_VideoEncoder);
        m_Graph.addComponent(intrusive_ptr<media::Component>(&m_RawWriter));
        
        m_Graph.connect(intrusive_ptr<media::Component>(&m_Capture), m_VideoEncoder);
        m_Graph.connect(m_VideoEncoder, intrusive_ptr<media::Component>(&m_RawWriter));
        
        return true;
    }
    
    void CameraServer::cleanGraph()
    {
        m_Graph.disconnect(m_VideoEncoder, intrusive_ptr<media::Component>(&m_RawWriter));
        m_Graph.disconnect(intrusive_ptr<media::Component>(&m_Capture), m_VideoEncoder);
        
        m_Graph.removeComponent(intrusive_ptr<media::Component>(&m_RawWriter));
        m_Graph.removeComponent(m_VideoEncoder);
        m_Graph.removeComponent(intrusive_ptr<media::Component>(&m_Capture));
        
        m_RawWriter.setStream(intrusive_ptr<base::IOStream>());
    }
    
    bool CameraServer::startGraph(asio::yield_context &yctx)
    {
        m_RawWriter.setVideoSampleFormat(
            media::VideoSampleFormat(
                m_VideoEncoder->getSampleType(),
                m_Capture.getCamera()->getPreviewFPS(),
                m_Capture.getCamera()->getVideoSize()
            )
        );

        base::YieldEvent startEvent(m_Runloop->ios());
        system::error_code se = startEvent.waitBlock(yctx, [this, &startEvent]() -> system::error_code
        {
            m_Graph.start([&startEvent](system::error_code e)
            {
                startEvent.set(e);
            });
            return base::makeErrorCode(base::kENoError);
        });

        if(se)
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::startGraph: failed to start the graph: " << se;
        }
        return !se;
    }
    
    void CameraServer::stopGraph(asio::yield_context &yctx)
    {
        base::YieldEvent stopEvent(m_Runloop->ios());
        system::error_code se = stopEvent.waitBlock(yctx, [this, &stopEvent]() -> system::error_code
        {
            m_Graph.stop([&stopEvent](system::error_code e)
            {
                stopEvent.set(e);
            });
            return base::makeErrorCode(base::kENoError);
        });

        if(se)
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::startGraph: failed to stop the graph: " << se;
        }
    }
    
    
    void CameraServer::startAccept()
    {
        m_Acceptor.async_accept(m_Socket,
            bind(&CameraServer::handleAccept, this,
                asio::placeholders::error
            )
        );
    }
    
    
    void CameraServer::handleAccept(system::error_code err)
    {
        if(err)
        {
            UQUAD_COMM_LOG(Error) << "CameraServer::handleAccept: e=" << err;
            if(m_State == kStateStarted)
            {
                startAccept();
            }
            return;
        }
        
        UQUAD_COMM_LOG(Debug) << "CameraServer::handleAccept: connected!";
        m_Socket.set_option(asio::ip::tcp::no_delay(true), err);

        m_SocketStream.setSocket(std::move(m_Socket));
        m_Graph.resume([this](system::error_code e)
        {
            if(e)
            {
                UQUAD_COMM_LOG(Error) << "CameraServer::handleAccept: could not resume graph, e" << e;
                system::error_code se;
                m_Socket.cancel(se);
                m_Socket.close(se);
                m_Socket.shutdown(asio::ip::tcp::socket::shutdown_both, se);
                startAccept();
                return;
            }
            m_State = kStateConnected;
            UQUAD_COMM_LOG(Debug) << "CameraServer::handleAccept: graph resumed!";
        });
    }
    
    
} //namespace comm
} //namespace uquad
