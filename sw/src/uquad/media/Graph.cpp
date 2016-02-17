#include "Graph.h"
#include "../base/Error.h"
#include "Log.h"
#include "Sink.h"
#include "Source.h"
#include <future>




namespace uquad
{
namespace media
{
    Graph::Graph(intrusive_ptr<base::Runloop> const &rl)
        : base::RefCounted()
        , m_State(kStateIdle)
        , m_Runloop(rl)
        , m_IOSStrand(m_Runloop->ios())
        , m_Components()
        , m_ExecutionOrder()
        , m_bInRun(false)
        , m_PendingExecutionsCount(0)
    {
    }
    
    Graph::~Graph()
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
                se.get_future().wait();
            }
            
        }
    }
    
    
    system::error_code Graph::addComponent(intrusive_ptr<Component> const &comp)
    {
        if(!comp)
            return base::makeErrorCode(base::kEInvalidArgument);
        
        if(comp->m_Graph)
            return base::makeErrorCode(base::kEAlreadyExists);
        
        if(m_State)
            return base::makeErrorCode(base::kENotApplicable);
        
        if(m_Components.count(comp))
            return base::makeErrorCode(base::kEAlreadyExists);
        
        comp->m_Graph = intrusive_ptr<Graph>(this);
        m_Components.insert(comp);
        
        if(intrusive_ptr<Generator> genComp = dynamic_pointer_cast<Generator>(comp))
        {
            genComp->generatorDelegate() += this;
        }
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code Graph::removeComponent(intrusive_ptr<Component> const &comp)
    {
        if(!comp)
            return base::makeErrorCode(base::kEInvalidArgument);
        
        if(m_State)
            return base::makeErrorCode(base::kENotApplicable);
        
        if(!m_Components.count(comp))
            return base::makeErrorCode(base::kENotFound);
        
        if(intrusive_ptr<Sink> sink = dynamic_pointer_cast<Sink>(comp))
        {
            for(std::size_t iip = 0; iip < sink->numInputPorts(); ++iip)
            {
                if(intrusive_ptr<InputPort> inputPort = sink->getInputPort(iip))
                {
                    if(intrusive_ptr<OutputPort> incoming = inputPort->getIncomingPort())
                    {
                        incoming->removeOutgoingPort(inputPort);
                    }
                }
            }
        }
        
        if(intrusive_ptr<Source> source = dynamic_pointer_cast<Source>(comp))
        {
            for(std::size_t iop = 0; iop < source->numOutputPorts(); ++iop)
            {
                if(intrusive_ptr<OutputPort> outputPort = source->getOutputPort(iop))
                {
                    outputPort->clearOutgoingPorts();
                }
            }
        }
        
        if(intrusive_ptr<Generator> genComp = dynamic_pointer_cast<Generator>(comp))
        {
            genComp->generatorDelegate() -= this;
        }
        
        comp->m_Graph.reset();
        m_Components.erase(comp);
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code Graph::connect(intrusive_ptr<OutputPort> const &op, intrusive_ptr<InputPort> const &ip)
    {
        if(!ip || !op)
            return base::makeErrorCode(base::kEInvalidArgument);
        
        if(m_State)
            return base::makeErrorCode(base::kENotApplicable);
        
        if(!m_Components.count(ip->getSink()) || !m_Components.count(op->getSource()))
            return base::makeErrorCode(base::kENotApplicable);
        
        if(hasConnection(ip->getSink(), op->getSource()))
           return base::makeErrorCode(base::kENotApplicable);
        
        return op->addOutgoingPort(ip);
    }
    
    system::error_code Graph::disconnect(intrusive_ptr<OutputPort> const &op, intrusive_ptr<InputPort> const &ip)
    {
        if(!ip || !op)
            return base::makeErrorCode(base::kEInvalidArgument);
        
        if(m_State)
            return base::makeErrorCode(base::kENotApplicable);
        
        if(!m_Components.count(ip->getSink()) || !m_Components.count(op->getSource()))
            return base::makeErrorCode(base::kENotApplicable);
        
        return op->removeOutgoingPort(ip);
    }
    
    system::error_code Graph::connect(intrusive_ptr<Component> const &from, intrusive_ptr<Component> const &to)
    {
        intrusive_ptr<Source> source = dynamic_pointer_cast<Source>(from);
        intrusive_ptr<Sink> sink = dynamic_pointer_cast<Sink>(to);
        if(!source || !sink)
            return base::makeErrorCode(base::kEInvalidArgument);
        
        if(m_State)
            return base::makeErrorCode(base::kENotApplicable);
        
        if(!m_Components.count(from) || !m_Components.count(to))
            return base::makeErrorCode(base::kENotApplicable);
        
        std::size_t numConnected = 0;
        for(std::size_t iop = 0; iop < source->numOutputPorts(); ++iop)
        {
            intrusive_ptr<OutputPort> op = source->getOutputPort(iop);
            for(std::size_t iip = 0; iip < sink->numInputPorts(); ++iip)
            {
                intrusive_ptr<InputPort> ip = sink->getInputPort(iip);
                
                if(!connect(op, ip))
                {
                    numConnected++;
                }
            }
        }
        
        return base::makeErrorCode(numConnected?base::kENoError:base::kENotApplicable);
    }
    
    system::error_code Graph::disconnect(intrusive_ptr<Component> const &from, intrusive_ptr<Component> const &to)
    {
        intrusive_ptr<Source> source = dynamic_pointer_cast<Source>(from);
        intrusive_ptr<Sink> sink = dynamic_pointer_cast<Sink>(to);
        if(!source || !sink)
            return base::makeErrorCode(base::kEInvalidArgument);
        
        if(m_State)
            return base::makeErrorCode(base::kENotApplicable);
        
        if(!m_Components.count(from) || !m_Components.count(to))
            return base::makeErrorCode(base::kENotApplicable);
        
        std::size_t numDisconnected = 0;
        for(std::size_t iop = 0; iop < source->numOutputPorts(); ++iop)
        {
            intrusive_ptr<OutputPort> op = source->getOutputPort(iop);
            for(std::size_t iip = 0; iip < sink->numInputPorts(); ++iip)
            {
                intrusive_ptr<InputPort> ip = sink->getInputPort(iip);
                
                if(!disconnect(op, ip))
                {
                    numDisconnected++;
                }
            }
        }
        return base::makeErrorCode(numDisconnected?base::kENoError:base::kENotApplicable);
    }
    
    void Graph::start()
    {
        asio::spawn(m_IOSStrand, bind(&Graph::doStart, this, _1, [](system::error_code){}));
    }
    
    void Graph::start(handler_t handler)
    {
        asio::spawn(m_IOSStrand, bind(&Graph::doStart, this, _1, std::move(handler)));
    }
    
    bool Graph::isStarted() const
    {
        return m_State > kStateIdle;
    }
    
    void Graph::stop()
    {
        asio::spawn(m_IOSStrand, bind(&Graph::doStop, this, _1, [](system::error_code){}));
    }
    
    void Graph::stop(handler_t handler)
    {
        asio::spawn(m_IOSStrand, bind(&Graph::doStop, this, _1, std::move(handler)));
    }
    
    void Graph::resume()
    {
        asio::spawn(m_IOSStrand, bind(&Graph::doResume, this, _1, [](system::error_code){}));
    }
    
    void Graph::resume(handler_t handler)
    {
        asio::spawn(m_IOSStrand, bind(&Graph::doResume, this, _1, std::move(handler)));
    }
    
    bool Graph::isRunning() const
    {
        return m_State == kStateStartedRunning;
    }
    
    void Graph::pause()
    {
        asio::spawn(m_IOSStrand, bind(&Graph::doPause, this, _1, [](system::error_code ){}));
    }
    
    void Graph::pause(handler_t handler)
    {
        asio::spawn(m_IOSStrand, bind(&Graph::doPause, this, _1, std::move(handler)));
    }
    
    bool Graph::hasConnection(intrusive_ptr<Component> const &from, intrusive_ptr<Component> const &to) const
    {
        if(intrusive_ptr<Source> source = dynamic_pointer_cast<Source>(from))
        {
            for(std::size_t iop = 0; iop < source->numOutputPorts(); ++iop)
            {
                if(intrusive_ptr<OutputPort> outputPort = source->getOutputPort(iop))
                {
                    for(auto ito = outputPort->getOutgoingPorts().begin(); ito != outputPort->getOutgoingPorts().end(); ++ito)
                    {
                        if((*ito)->getSink() == to)
                            return true;
                        
                        if(hasConnection((*ito)->getSink(), to))
                            return true;
                    }
                }
            }
        }
        return false;
    }
    
    bool Graph::buildExecutionOrder()
    {
        if(m_Components.empty())
            return false;
        
        m_ExecutionOrder.clear();
        
        unordered_set< intrusive_ptr<Component> > remaining(m_Components);
        std::list< intrusive_ptr<Component> > current;
        std::for_each(m_Components.begin(), m_Components.end(), [&current, &remaining](intrusive_ptr<Component> const &comp)
        {
            if(intrusive_ptr<Generator> genComp = dynamic_pointer_cast<Generator>(comp))
            {
                remaining.erase(comp);
                current.push_front(comp);
            }
        });
        
        while(!current.empty())
        {
            
            auto last = current.end();
            auto icomp = current.begin();
            while(icomp != last)
            {
                intrusive_ptr<Source> source = dynamic_pointer_cast<Source>(*icomp);
                if(source)
                {
                    for(std::size_t iop = 0; iop < source->numOutputPorts(); ++iop)
                    {
                        intrusive_ptr<OutputPort> op = source->getOutputPort(iop);
                        if(!op)
                            continue;
                        
                        std::for_each(op->getOutgoingPorts().begin(), op->getOutgoingPorts().end(), [&remaining, &current, &last](intrusive_ptr<InputPort> const &ip)
                        {
                            intrusive_ptr<Sink> sink = ip->getSink();
                            if(remaining.count(sink))
                            {
                                last = current.insert(last, sink);
                                remaining.erase(sink);
                            }
                        });
                    }
                }
                m_ExecutionOrder.push_back(*icomp);
                icomp = current.erase(icomp);
            }
        }
        
        return !m_ExecutionOrder.empty();
    }
    
    void Graph::doStart(asio::yield_context yield, handler_t handler)
    {
        switch(m_State)
        {
        case kStateIdle:
            if(buildExecutionOrder())
            {
                if(prepareComponents(yield))
                {
                    m_State = kStateStartedPaused;
                }
                else
                {
                    UQUAD_MEDIA_LOG(Error) << "Graph::doStart: prepareComponents() failed!";
                    handler(base::makeErrorCode(base::kENotStarted));
                    return;
                }
            }
            else
            {
                UQUAD_MEDIA_LOG(Error) << "Graph::doStart: buildExecutionOrder() failed!";
                handler(base::makeErrorCode(base::kENotStarted));
                return;
            }
            break;
        
        case kStateStartedPaused:
        case kStateStartedRunning:
            {
                UQUAD_MEDIA_LOG(Error) << "Graph::doStart: is in invalid state!";
                handler(base::makeErrorCode(base::kEInvalidState));
                return;
            }
            break;
        }
        handler(base::makeErrorCode(base::kENoError));
    }
    
    void Graph::doStop(asio::yield_context yield, handler_t handler)
    {
        switch(m_State)
        {
        case kStateIdle:
            {
                UQUAD_MEDIA_LOG(Error) << "Graph::doStop: is in invalid state!";
                handler(base::makeErrorCode(base::kENotStarted));
                return;
            }
            break;
        
        case kStateStartedPaused:
            {
                m_State = kStateIdle;
                unprepareComponents(yield);
            }
            break;
            
        case kStateStartedRunning:
            {
                doPause(yield, [](system::error_code){});
                m_State = kStateIdle;
                unprepareComponents(yield);
            }
            break;
        }
        handler(base::makeErrorCode(base::kENoError));
    }
    
    void Graph::doResume(asio::yield_context yield, handler_t handler)
    {
        switch(m_State)
        {
        case kStateIdle:
            {
                UQUAD_MEDIA_LOG(Error) << "Graph::doResume: is in invalid state!";
                handler(base::makeErrorCode(base::kENotStarted));
                return;
            }
            break;
        
        case kStateStartedPaused:
            {
                m_State = kStateStartedRunning;
                if(!resumeComponents(yield))
                {
                    UQUAD_MEDIA_LOG(Error) << "Graph::doResume: resumeComponents() failed!";
                    handler(base::makeErrorCode(base::kENotStarted));
                    m_State = kStateStartedPaused;
                    return;
                }
            }
            break;
            
        case kStateStartedRunning:
            {
                UQUAD_MEDIA_LOG(Warning) << "Graph::doResume: is already running!";
                handler(base::makeErrorCode(base::kEAlreadyStarted));
                return;
            }
            break;
            
        default:
            break;
        }
        handler(base::makeErrorCode(base::kENoError));
    }
    
    void Graph::doPause(asio::yield_context yield, handler_t handler)
    {
        switch(m_State)
        {
        case kStateIdle:
        case kStateStartedPaused:
            {
                UQUAD_MEDIA_LOG(Error) << "Graph::doPause: is in invalid state!";
                handler(base::makeErrorCode(base::kENotStarted));
                return;
            }
            break;
            
        case kStateStartedRunning:
            m_State = kStateStartedPaused;
            pauseComponents(yield);
            break;
        }
        handler(base::makeErrorCode(base::kENoError));
    }
    
    bool Graph::prepareComponents(asio::yield_context yield)
    {
        for(auto icomp = m_ExecutionOrder.begin(); icomp != m_ExecutionOrder.end(); ++icomp)
        {
            if((*icomp)->prepare(yield))
            {
                for(auto icomp2 = m_ExecutionOrder.begin(); icomp2 != icomp; ++icomp2)
                {
                    (*icomp2)->unprepare(yield);
                }
                UQUAD_MEDIA_LOG(Error) << "Graph::prepareComponents: failed to prepare component!";
                return false;
            }
        }
        m_bInRun = false;
        m_PendingExecutionsCount = 0;
        return true;
    }
    
    void Graph::unprepareComponents(asio::yield_context yield)
    {
        for(auto icomp = m_ExecutionOrder.begin(); icomp != m_ExecutionOrder.end(); ++icomp)
        {
            (*icomp)->unprepare(yield);
        }
        m_bInRun = false;
        m_PendingExecutionsCount = 0;
    }
    
    bool Graph::resumeComponents(asio::yield_context yield)
    {
        for(auto icomp = m_ExecutionOrder.begin(); icomp != m_ExecutionOrder.end(); ++icomp)
        {
            if((*icomp)->resume(yield))
            {
                for(auto icomp2 = m_ExecutionOrder.begin(); icomp2 != icomp; ++icomp2)
                {
                    (*icomp2)->pause(yield);
                }
                UQUAD_MEDIA_LOG(Error) << "Graph::resumeComponents: failed to resume component!";
                return false;
            }
        }
        m_PendingExecutionsCount = 0;
        return true;
    }
    
    void Graph::pauseComponents(asio::yield_context yield)
    {
        for(auto icomp = m_ExecutionOrder.begin(); icomp != m_ExecutionOrder.end(); ++icomp)
        {
            (*icomp)->pause(yield);
        }
    }
    
    void Graph::run()
    {
        if(m_bInRun)
        {
            m_Runloop->ios().post([this]() -> void
            {
                asio::spawn(m_IOSStrand, bind(&Graph::doRun, this, _1));
            });
        }
    }
    
    void Graph::doRun(asio::yield_context yield)
    {
        if(!isRunning())
        {
            m_bInRun = false;
            m_PendingExecutionsCount = 0;
            return;
        }
        
        auto icomp = m_ExecutionOrder.begin();
        
        while(icomp != m_ExecutionOrder.end())
        {
            system::error_code ee = (*icomp)->run(yield);
            
            if(ee)
            {
                UQUAD_MEDIA_LOG(Error) << "Graph::doRun: component::run() failed with error: " << ee;
                break;
            }
            
            icomp++;
        }
        
        for(auto icomp2 = m_ExecutionOrder.begin(); icomp2 != icomp; ++icomp2)
        {
            (*icomp2)->rest(yield);
        }
        
        if(m_PendingExecutionsCount)
        {
            m_PendingExecutionsCount--;
            run();
        }
        else
        {
            m_bInRun = false;
        }
    }
    
    void Graph::onSampleGenerated()
    {
        if(!isRunning())
            return;

        if(m_bInRun)
        {
            m_PendingExecutionsCount++;
        }
        else
        {
            m_bInRun = true;
            run();
        }
    }
    
} //namespace media
} //namespace uquad
