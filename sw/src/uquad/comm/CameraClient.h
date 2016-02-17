#ifndef UQUAD_COMM_CAMERA_CLIENT_H
#define UQUAD_COMM_CAMERA_CLIENT_H

#include "Config.h"
#include "../base/RefCounted.h"
#include "../base/Delegate.h"
#include "../media/Graph.h"
#include "../media/Playout.h"
#include "../media/VideoDecoder.h"
#include "../media/RawReader.h"
#include "../base/IOSocketStream.h"
#include "../base/Delegate.h"


namespace uquad
{
namespace comm
{
    class CameraClientDelegate
	{
	public:
		CameraClientDelegate() = default;
		virtual ~CameraClientDelegate() = default;
        
        virtual void onCameraClientConnected() {}
        virtual void onCameraClientDisconnected() {}
	};
    
    class CameraClient
        : public base::RefCounted
        , public base::IOAsyncSocketStreamDelegate
    {
    public:
        typedef function<void (system::error_code)> handler_t;
        
        typedef base::Delegate<CameraClientDelegate> camera_client_delegate_t;
        
        typedef enum
        {
            kStateIdle = 0,
            kStateStarted,
            kStateConnected
        } eState;
        
        
        CameraClient(intrusive_ptr<base::Runloop> const &rl);
        ~CameraClient();
        
        inline asio::ip::address const& getRemoteAddress() const { return m_RemoteAddress; }
        system::error_code setRemoteAddress(asio::ip::address const &addr);
        
        inline int getRemotePort() { return m_RemotePort; }
        system::error_code setRemotePort(int port);
        
        inline Vec2i getRemoteVideoSampleSize() const { return m_RemoteVideoSampleSize; }
        system::error_code setRemoteVideoSampleSize(Vec2i vs);
        
        inline float getRemoteVideoSampleRate() const { return m_RemoteVideoSampleRate; }
        system::error_code setRemoteVideoSampleRate(float vsr);
        
        
        intrusive_ptr<media::Playout> getPlayout();
        
        void start();
        void start(handler_t handler);
        bool isStarted() const;
        void stop();
        void stop(handler_t handler);
        
        void connect();
        void connect(handler_t handler);
        bool isConnected() const;
        void disconnect();
        void disconnect(handler_t handler);
        
        inline camera_client_delegate_t const& cameraClientDelegate() { return m_CameraClientDelegate; }
        
    protected:
        inline void notifyOnCameraClientConnected()
        {
            m_CameraClientDelegate.call(&CameraClientDelegate::onCameraClientConnected);
        }
        
        inline void notifyOnCameraClientDisconnected()
        {
            m_CameraClientDelegate.call(&CameraClientDelegate::onCameraClientDisconnected);
        }
        
    private:
        void doStart(asio::yield_context yctx, handler_t handler);
        void doStop(asio::yield_context yctx, handler_t handler);
        
        bool makeGraph();
        void cleanGraph();
        
        bool startGraph(asio::yield_context &yctx);
        void stopGraph(asio::yield_context &yctx);
        
        void doConnect(asio::yield_context yield, handler_t handler);
        void doDisconnect(asio::yield_context yield, handler_t handler);
        
        void handleConnect(system::error_code err, handler_t handler);
        
        void onIOAsyncSocketStreamError(system::error_code const &e);
        
    private:
        camera_client_delegate_t m_CameraClientDelegate;
        atomic<eState> m_State;
        intrusive_ptr<base::Runloop> m_Runloop;
        asio::io_service::strand m_IOSStrand;
        asio::ip::tcp::resolver m_Resolver;
        asio::ip::tcp::socket m_Socket;
        
        asio::ip::address m_RemoteAddress;
        int m_RemotePort;
        
        Vec2i m_RemoteVideoSampleSize;
        float m_RemoteVideoSampleRate;
        
        media::RawReader m_RawReader;
        intrusive_ptr<media::VideoDecoder> m_VideoDecoder;
        media::Playout m_Playout;
        media::Graph m_Graph;
        base::IOAsyncSocketStream m_SocketStream;
    };
    
    
} //namespace comm
} //namespace uquad

#endif //UQUAD_COMM_CAMERA_CLIENT_H