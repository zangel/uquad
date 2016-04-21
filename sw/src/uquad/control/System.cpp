#include "System.h"
#include "Block.h"
#include "Signal.h"

namespace uquad
{
namespace control
{
    System::System(intrusive_ptr<base::Runloop> const &rl)
        : base::Graph()
        , BaseService()
        , m_SM(ref(*this))
        , m_Runloop(rl)
        , m_IOSStrand(m_Runloop->ios())
        , m_BlockUpdateRequests()
    {
    }
    
    System::~System()
    {
    }
    
    bool System::isMutable() const
    {
        return true;
    }
    
    system::error_code System::addBlock(intrusive_ptr<Block> const &block)
    {
        return base::Graph::addNode(block);
    }
    
    system::error_code System::removeBlock(intrusive_ptr<Block> const &block)
    {
        return base::Graph::removeNode(block);
    }
    
    system::error_code System::connectSignals(intrusive_ptr<OutputSignal> const &os, intrusive_ptr<InputSignal> const &is)
    {
        return base::Graph::connectPorts(os, is);
    }
    
    system::error_code System::disconnectSignals(intrusive_ptr<OutputSignal> const &os, intrusive_ptr<InputSignal> const &is)
    {
        return base::Graph::disconnectPorts(os, is);
    }
    
    system::error_code System::connectBlocks(intrusive_ptr<Block> src, intrusive_ptr<Block> dst)
    {
        return base::Graph::connectNodes(src, dst);
    }
    
    
    bool System::isUpdating() const
    {
        return m_SM.template is_flag_active<FUpdating>();
    }
    
    void System::update()
    {
        asio::spawn(
            m_IOSStrand,
            [this](asio::yield_context yctx)
            {
                m_SM.process_event(EUpdate(handler_t(), yctx));
            }
        );
    }
    
    void System::update(handler_t handler)
    {
        asio::spawn(
            m_IOSStrand,
            [this, handler](asio::yield_context yctx)
            {
                m_SM.process_event(EUpdate(handler, yctx));
            }
        );
    }
    
    void System::requestBlockUpdate(intrusive_ptr<Block> const &block)
    {
        m_BlockUpdateRequests.insert(block);
    }
    
    system::error_code System::doStart(asio::yield_context yctx)
    {
        system::error_code van = visitAllNodes(
            [yctx](intrusive_ptr<base::Graph::Node> node) -> system::error_code
            {
                if(intrusive_ptr<Block> block = dynamic_pointer_cast<Block>(node))
                {
                    return block->prepare(yctx);
                }
                return base::makeErrorCode(base::kEInvalidArgument);
            },
            [yctx](intrusive_ptr<base::Graph::Node> node) -> system::error_code
            {
                if(intrusive_ptr<Block> block = dynamic_pointer_cast<Block>(node))
                {
                    block->unprepare(yctx);
                    return base::makeErrorCode(base::kENoError);
                }
                return base::makeErrorCode(base::kEInvalidArgument);
            }
        );
        
        if(van)
            return van;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code System::doStop(asio::yield_context yctx)
    {
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code System::doUpdate(asio::yield_context yctx)
    {
        unordered_set< intrusive_ptr<base::Graph::Node> > nodes(
            m_BlockUpdateRequests.begin(),
            m_BlockUpdateRequests.end()
        );
        
        m_BlockUpdateRequests.clear();
        
        visit(
            nodes,
            [yctx](intrusive_ptr<base::Graph::Node> node) -> system::error_code
            {
                if(intrusive_ptr<Block> block = dynamic_pointer_cast<Block>(node))
                {
                    return block->update(yctx);
                }
                return base::makeErrorCode(base::kEInvalidArgument);
            },
            [](intrusive_ptr<base::Graph::Connection> conn) -> system::error_code
            {
                intrusive_ptr<OutputSignal> os = dynamic_pointer_cast<OutputSignal>(conn->sourcePort());
                intrusive_ptr<InputSignal> is = dynamic_pointer_cast<InputSignal>(conn->destinationPort());
                if(os && is)
                {
                    return is->assign(os);
                }
                
                return base::makeErrorCode(base::kEInvalidArgument);
            }
        );
        m_BlockUpdateRequests.clear();
        return base::makeErrorCode(base::kENoError);
    }
    

} //namespace control
} //namespace uquad
