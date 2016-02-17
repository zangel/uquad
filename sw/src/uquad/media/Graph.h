#ifndef UQUAD_MEDIA_GRAPH_H
#define UQUAD_MEDIA_GRAPH_H

#include "Config.h"
#include "../base/RefCounted.h"
#include "../base/Runloop.h"
#include "Component.h"
#include "Port.h"
#include "Generator.h"

namespace uquad
{
namespace media
{
    class Graph
    	: public base::RefCounted
        , public GeneratorDelegate
    {
    public:
        typedef function<void (system::error_code)> handler_t;
        
    	Graph(intrusive_ptr<base::Runloop> const &rl);
    	~Graph();
        
        typedef enum
        {
            kStateIdle = 0,
            kStateStartedPaused,
            kStateStartedRunning
        } eState;
        
        system::error_code addComponent(intrusive_ptr<Component> const &comp);
        system::error_code removeComponent(intrusive_ptr<Component> const &comp);
        
        system::error_code connect(intrusive_ptr<OutputPort> const &op, intrusive_ptr<InputPort> const &ip);
        system::error_code disconnect(intrusive_ptr<OutputPort> const &op, intrusive_ptr<InputPort> const &ip);
        
        system::error_code connect(intrusive_ptr<Component> const &from, intrusive_ptr<Component> const &to);
        system::error_code disconnect(intrusive_ptr<Component> const &from, intrusive_ptr<Component> const &to);
        
        void start();
        void start(handler_t handler);
        bool isStarted() const;
        void stop();
        void stop(handler_t handler);
        
        void resume();
        void resume(handler_t handler);
        bool isRunning() const;
        void pause();
        void pause(handler_t handler);
        
        inline intrusive_ptr<base::Runloop> getRunloop() const { return m_Runloop; }
        
    private:
        bool hasConnection(intrusive_ptr<Component> const &from, intrusive_ptr<Component> const &to) const;
        bool buildExecutionOrder();
        
        void doStart(asio::yield_context yield, handler_t handler);
        void doStop(asio::yield_context yield, handler_t handler);
        void doResume(asio::yield_context yield, handler_t handler);
        void doPause(asio::yield_context yield, handler_t handler);
        
        bool prepareComponents(asio::yield_context yield);
        void unprepareComponents(asio::yield_context yield);
        
        bool resumeComponents(asio::yield_context yield);
        void pauseComponents(asio::yield_context yield);
        
        
        void run();
        void doRun(asio::yield_context yield);
        
    protected:
        void onSampleGenerated();
        
    private:
        eState m_State;
        intrusive_ptr<base::Runloop> m_Runloop;
        asio::io_service::strand m_IOSStrand;
        unordered_set< intrusive_ptr<Component> > m_Components;
        std::vector< intrusive_ptr<Component> > m_ExecutionOrder;
        atomic<bool> m_bInRun;
        atomic<std::size_t> m_PendingExecutionsCount;
    };

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_GRAPH_H