#include "Block.h"
#include "Signal.h"
#include "System.h"

namespace uquad
{
namespace control
{
    Block::Block()
        : base::RefCounted()
        , base::Object()
        , base::Graph::Node()
    {
    }
    
    Block::~Block()
    {
    }
    
    intrusive_ptr<System> Block::system() const
    {
        return dynamic_pointer_cast<System>(graph());
    }
    
    void Block::requestUpdate()
    {
        if(intrusive_ptr<System> sys = system())
        {
            sys->requestBlockUpdate(intrusive_ptr<Block>(this));
        }
    }
    
    system::error_code Block::prepare(asio::yield_context yctx)
    {
        if(intrusive_ptr<System> sys = system())
            return sys->isStarted()?base::makeErrorCode(base::kENotApplicable):base::makeErrorCode(base::kENoError);
        
        return base::makeErrorCode(base::kENotApplicable);
    }
    
    bool Block::isPrepared() const
    {
        if(intrusive_ptr<System> sys = system())
            return sys->isStarted();
        
        return false;
    }
    
    void Block::unprepare(asio::yield_context yctx)
    {
        
    }
    
    system::error_code Block::update(asio::yield_context yctx)
    {
        return base::makeErrorCode(base::kENoError);
    }
    
} //namespace control
} //namespace uquad
