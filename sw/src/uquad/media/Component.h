#ifndef UQUAD_MEDIA_COMPONENT_H
#define UQUAD_MEDIA_COMPONENT_H

#include "Config.h"

#include "../base/RefCounted.h"
#include "../base/Delegate.h"

namespace uquad
{
namespace media
{
	class ComponentFactory;
    class Graph;
    class Component;
    
	class ComponentDelegate
	{
	public:
		ComponentDelegate() = default;
		virtual ~ComponentDelegate() = default;
	};
    
    class Component
    	: public base::RefCounted
    {
    public:
        typedef std::type_index Type;
        friend class Graph;
        
        class Registry
            : public base::RefCounted
        {
        public:
            Registry(std::string const &n, Type const &t);
            ~Registry();
            
            virtual intrusive_ptr<Component> createComponent() const = 0;
            
            template <typename T>
            inline intrusive_ptr<T> createComponent() const { return dynamic_pointer_cast<T>(createComponent()); }
            
            std::string name;
            Type type;
        };
        
    	typedef base::Delegate<ComponentDelegate> component_delegate_t;

    	Component();
    	~Component();
        
        virtual bool isValid() const;
        
        virtual system::error_code prepare(asio::yield_context &yctx);
        bool isPrepared() const;
        virtual void unprepare(asio::yield_context &yctx);
        
        virtual system::error_code resume(asio::yield_context &yctx);
        bool isRunning() const;
        virtual void pause(asio::yield_context &yctx);
        
        
        virtual system::error_code run(asio::yield_context &yctx);
        virtual void rest(asio::yield_context &yctx);
        
        inline intrusive_ptr<Graph> const& getGraph() { return m_Graph; }
        
        
        template <typename C>
    	static system::error_code registerComponents(ComponentFactory &f);

    	inline component_delegate_t const& componentDelegate() { return m_ComponentDelegate; }
        
    protected:
    
    protected:
    	component_delegate_t m_ComponentDelegate;
        
    private:
        intrusive_ptr<Graph> m_Graph;
    };

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_COMPONENT_H
