#include "Component.h"
#include "Graph.h"
#include "../base/Error.h"

namespace uquad
{
namespace media
{
    Component::Registry::Registry(std::string const &n, Component::Type const &t)
        : name(n)
        , type(t)
    {
    }
    
    Component::Registry::~Registry()
    {
    }


	Component::Component()
        : base::RefCounted()
        , m_ComponentDelegate()
        , m_Graph()
	{
	}

	Component::~Component()
	{
	}
    
    bool Component::isValid() const
    {
        return true;
    }
        
    system::error_code Component::prepare(asio::yield_context &yctx)
    {
        return base::makeErrorCode(base::kENoError);
    }
    
    bool Component::isPrepared() const
    {
        return m_Graph && m_Graph->isStarted();
    }
    
    void Component::unprepare(asio::yield_context &yctx)
    {
    }
    
    system::error_code Component::resume(asio::yield_context &yctx)
    {
        return base::makeErrorCode(base::kENoError);
    }
    
    void Component::pause(asio::yield_context &yctx)
    {
    }
    
    system::error_code Component::run(asio::yield_context &yctx)
    {
        return base::makeErrorCode(base::kENoError);
    }
    
    bool Component::isRunning() const
    {
        return m_Graph && m_Graph->isRunning();
    }
    
    void Component::rest(asio::yield_context &yctx)
    {
    }

} //namespace media
} //namespace uquad
