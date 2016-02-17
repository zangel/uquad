#ifndef UQUAD_COMM_CAMERA_SERVER_H
#define UQUAD_COMM_CAMERA_SERVER_H

#include "Config.h"
#include "../base/RefCounted.h"
#include "../base/Delegate.h"
#include "../media/Graph.h"
#include "../hal/Camera.h"
#include "../media/Capture.h"
#include "../media/VideoEncoder.h"
#include "../media/RawWriter.h"
#include "../base/IOSocketStream.h"



namespace uquad
{
namespace comm
{
    class CameraServer
        : public base::RefCounted
        , protected base::IOAsyncSocketStreamDelegate
    {
    public:
        typedef function<void (system::error_code)> handler_t;
        
        typedef enum
        {
            kStateIdle = 0,
            kStateStarted,
            kStateConnected
        } eState;
        
        
        CameraServer(intrusive_ptr<base::Runloop> const &rl);
        ~CameraServer();
        
        inline asio::ip::address const& getListenAddress() const { return m_ListenAddress; }
        system::error_code setListenAddress(asio::ip::address const &addr);
        
        inline int getListenPort() { return m_ListenPort; }
        system::error_code setListenPort(int port);

        intrusive_ptr<hal::Camera> const& getCamera() const;
        system::error_code setCamera(intrusive_ptr<hal::Camera> const &camera);
        
        void start();
        void start(handler_t handler);
        bool isStarted() const;
        bool isConnected() const;
        void stop();
        void stop(handler_t handler);
        
    protected:
        void onIOAsyncSocketStreamError(system::error_code const &e);
        
    private:
        void doStart(asio::yield_context yctx, handler_t handler);
        void doStop(asio::yield_context yctx, handler_t handler);
        
        bool makeGraph();
        void cleanGraph();
        
        bool startGraph(asio::yield_context &yctx);
        void stopGraph(asio::yield_context &yctx);
        
        void startAccept();
        void handleAccept(system::error_code err);
                
    private:
        atomic<eState> m_State;
        intrusive_ptr<base::Runloop> m_Runloop;
        asio::io_service::strand m_IOSStrand;
        asio::ip::tcp::resolver m_Resolver;
        asio::ip::tcp::acceptor m_Acceptor;
        asio::ip::tcp::socket m_Socket;
        asio::ip::address m_ListenAddress;
        int m_ListenPort;
        media::Capture m_Capture;
        intrusive_ptr<media::VideoEncoder> m_VideoEncoder;
        media::RawWriter m_RawWriter;
        media::Graph m_Graph;
        base::IOAsyncSocketStream m_SocketStream;
    };

} //namespace comm
} //namespace uquad

#endif //UQUAD_COMM_CAMERA_SERVER_H
