#include "CameraClient.h"
#include "../media/ComponentFactory.h"
#include "../base/Error.h"
#include "Log.h"
#include <future>

namespace uquad
{
namespace comm
{
    CameraClient::CameraClient(intrusive_ptr<base::Runloop> const &rl)
        : m_CameraClientDelegate()
        , m_State(kStateIdle)
        , m_Runloop(rl)
        , m_IOSStrand(rl->ios())
        , m_Resolver(rl->ios())
        , m_Socket(rl->ios())
        , m_RemoteAddress(asio::ip::address::from_string("127.0.0.1"))
        , m_RemotePort(35624)
        , m_RemoteVideoSampleSize(640, 480)
        , m_RemoteVideoSampleRate(30.0f)
        , m_RawReader()
        , m_VideoDecoder(SI(media::ComponentFactory).createVideoDecoder(media::kVideoCodecTypeH264))
        , m_Playout()
        , m_Graph(rl)
        , m_SocketStream()
    {
        intrusive_ptr_add_ref(&m_RawReader);
        intrusive_ptr_add_ref(&m_Playout);
        intrusive_ptr_add_ref(&m_Graph);
        intrusive_ptr_add_ref(&m_SocketStream);
        
        m_SocketStream.ioAsyncStreamDelegate() += this;
    }
    
    CameraClient::~CameraClient()
    {
        if(isConnected())
        {
            std::promise<system::error_code> se;
            disconnect([this, &se](system::error_code e) { se.set_value(e); });
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
    
    system::error_code CameraClient::setRemoteAddress(asio::ip::address const &addr)
    {
        if(isConnected())
            return base::makeErrorCode(base::kEInvalidState);
        
        m_RemoteAddress = addr;
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code CameraClient::setRemotePort(int port)
    {
        if(isConnected())
            return base::makeErrorCode(base::kEInvalidState);
        
        m_RemotePort = port;
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code CameraClient::setRemoteVideoSampleSize(Vec2i vs)
    {
        if(isConnected())
            return base::makeErrorCode(base::kEInvalidState);
        
        m_RemoteVideoSampleSize = vs;
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code CameraClient::setRemoteVideoSampleRate(float vsr)
    {
        if(isConnected())
            return base::makeErrorCode(base::kEInvalidState);
        
        m_RemoteVideoSampleRate = vsr;
        return base::makeErrorCode(base::kENoError);
    }
    
    intrusive_ptr<media::Playout> CameraClient::getPlayout()
    {
        return intrusive_ptr<media::Playout>(&m_Playout);
    }
    
    void CameraClient::start()
    {
        start([](system::error_code){});
    }
    
    void CameraClient::start(handler_t handler)
    {
        asio::spawn(m_IOSStrand, bind(&CameraClient::doStart, this, _1, handler));
    }
    
    bool CameraClient::isStarted() const
    {
        return m_State >= kStateStarted;
    }
    
    void CameraClient::stop()
    {
        stop([](system::error_code){});
    }
    
    void CameraClient::stop(handler_t handler)
    {
        asio::spawn(m_IOSStrand, bind(&CameraClient::doStop, this, _1, handler));
    }
    
    void CameraClient::connect()
    {
        connect([](system::error_code){});
    }
    
    void CameraClient::connect(handler_t handler)
    {
        asio::spawn(m_IOSStrand, bind(&CameraClient::doConnect, this, _1, handler));
    }
    
    bool CameraClient::isConnected() const
    {
        return m_State == kStateConnected;
    }
    
    void CameraClient::disconnect()
    {
        disconnect([](system::error_code){});
    }
    
    void CameraClient::disconnect(handler_t handler)
    {
        asio::spawn(m_IOSStrand, bind(&CameraClient::doDisconnect, this, _1, handler));
    }
    
    void CameraClient::doStart(asio::yield_context yctx, handler_t handler)
    {
        if(isStarted())
        {
            UQUAD_COMM_LOG(Error) << "CameraClient::doStart: already started!";
            handler(base::makeErrorCode(base::kEAlreadyStarted));
            return;
        }
        
        if(!makeGraph())
        {
            UQUAD_COMM_LOG(Error) << "CameraClient::doStart: failed to make the graph!";
            handler(base::makeErrorCode(base::kENotStarted));
            return;
        }
        
        if(!startGraph(yctx))
        {
            UQUAD_COMM_LOG(Error) << "CameraClient::doStart: failed to start the graph!";
            cleanGraph();
            handler(base::makeErrorCode(base::kENotStarted));
            return;
        }
        
        UQUAD_COMM_LOG(Debug) << "CameraClient::doStart: started successfully!";
        m_State = kStateStarted;
        handler(base::makeErrorCode(base::kENoError));
    }
    
    void CameraClient::doStop(asio::yield_context yctx, handler_t handler)
    {
        if(!isStarted())
        {
            UQUAD_COMM_LOG(Error) << "CameraClient::doStop: not started!";
            handler(base::makeErrorCode(base::kENotStarted));
            return;
        }
        
        if(isConnected())
        {
            doDisconnect(yctx, [](system::error_code){});
        }
        
        stopGraph(yctx);
        cleanGraph();
        
        m_State = kStateIdle;
        UQUAD_COMM_LOG(Debug) << "CameraClient::doStop: stopped successfully!";
        handler(base::makeErrorCode(base::kENoError));
    }
        
    void CameraClient::doConnect(asio::yield_context yield, handler_t handler)
    {
        if(!isStarted())
        {
            UQUAD_COMM_LOG(Error) << "CameraClient::doConnect: not started!";
            handler(base::makeErrorCode(base::kENotStarted));
            return;
        }
        
        if(isConnected())
        {
            UQUAD_COMM_LOG(Error) << "CameraClient::doConnect: already connected!";
            handler(base::makeErrorCode(base::kEAlreadyOpened));
            return;
        }
        
        system::error_code err;
        
        asio::ip::tcp::resolver::query hostQuery(m_RemoteAddress.to_string(), boost::lexical_cast<std::string>(m_RemotePort));
        asio::ip::tcp::resolver::iterator itEndpoint = m_Resolver.resolve(hostQuery, err);
        
        if(err || itEndpoint == asio::ip::tcp::resolver::iterator())
        {
            UQUAD_COMM_LOG(Error) << "CameraClient::doConnect: failed to resolve remote host!";
            handler(base::makeErrorCode(base::kENotFound));
            return;
        }
        
        m_Socket.async_connect(
            *itEndpoint,
            bind(
                &CameraClient::handleConnect, this,
                asio::placeholders::error,
                std::move(handler)
            )
        );
    }
    
    void CameraClient::doDisconnect(asio::yield_context yctx, handler_t handler)
    {
        if(!isConnected())
        {
            UQUAD_COMM_LOG(Error) << "CameraClient::doDisconnect: not connected!";
            handler(base::makeErrorCode(base::kENotOpened));
            return;
        }
        
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
            UQUAD_COMM_LOG(Error) << "CameraClient::doDisconnect: failed to pause the graph: " << pe;
            handler(base::makeErrorCode(base::kENotOpened));
            return;
        }
        
        m_SocketStream.close();
        UQUAD_COMM_LOG(Debug) << "CameraClient::doDisconnect: disconnected";
        m_State = kStateStarted;
        handler(base::makeErrorCode(base::kENoError));
    }
    
    bool CameraClient::makeGraph()
    {
        if(!m_VideoDecoder)
        {
            UQUAD_COMM_LOG(Error) << "CameraClient::makeGraph: no video decoder!";
            return false;
        }
        
        m_RawReader.setStream(intrusive_ptr<base::IOStream>(&m_SocketStream));
        m_RawReader.setVideoSampleFormat(
            media::VideoSampleFormat(
                m_VideoDecoder->getSampleType(),
                m_RemoteVideoSampleRate,
                m_RemoteVideoSampleSize
            )
        );
        
        m_Graph.addComponent(intrusive_ptr<media::Component>(&m_RawReader));
        m_Graph.addComponent(m_VideoDecoder);
        m_Graph.addComponent(intrusive_ptr<media::Component>(&m_Playout));
        
        m_Graph.connect(intrusive_ptr<media::Component>(&m_RawReader), m_VideoDecoder);
        m_Graph.connect(m_VideoDecoder, intrusive_ptr<media::Component>(&m_Playout));
        
        return true;
    }
    
    void CameraClient::cleanGraph()
    {
        m_Graph.disconnect(m_VideoDecoder, intrusive_ptr<media::Component>(&m_Playout));
        m_Graph.disconnect(intrusive_ptr<media::Component>(&m_RawReader), m_VideoDecoder);
        
        m_Graph.removeComponent(intrusive_ptr<media::Component>(&m_Playout));
        m_Graph.removeComponent(m_VideoDecoder);
        m_Graph.removeComponent(intrusive_ptr<media::Component>(&m_RawReader));
        
        m_RawReader.setStream(intrusive_ptr<base::IOStream>());
    }
    
    bool CameraClient::startGraph(asio::yield_context &yctx)
    {
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
            UQUAD_COMM_LOG(Error) << "CameraClient::startGraph: failed to start the graph: " << se;
        }
        
        return !se;
    }
    
    void CameraClient::stopGraph(asio::yield_context &yctx)
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
            UQUAD_COMM_LOG(Error) << "CameraClient::stopGraph: failed to stop the graph: " << se;
        }
    }
    
    void CameraClient::handleConnect(system::error_code err, handler_t handler)
    {
        if(err)
        {
            handler(base::makeErrorCode(base::kENotOpened));
            UQUAD_COMM_LOG(Error) << "CameraClient::handleConnect: could not connect:" << err;
            return;
        }
        
        m_SocketStream.setSocket(std::move(m_Socket));
        m_Graph.resume([this, handler](system::error_code re)
        {
            if(re)
            {
                handler(base::makeErrorCode(base::kENotOpened));
                UQUAD_COMM_LOG(Error) << "CameraClient::handleConnect: could not resume graph:" << re;
                return;
            }
            
            m_State = kStateConnected;
            handler(base::makeErrorCode(base::kENoError));
            notifyOnCameraClientConnected();
            UQUAD_COMM_LOG(Debug) << "CameraClient::handleConnect: connected";
        });
        
    }
    
    void CameraClient::onIOAsyncSocketStreamError(system::error_code const &e)
    {
        disconnect([this](system::error_code e)
        {
            notifyOnCameraClientDisconnected();
            UQUAD_COMM_LOG(Debug) << "CameraClient::onIOAsyncSocketStreamError: disconnected!";
        });
    }
    

} //namespace comm
} //namespace uquad