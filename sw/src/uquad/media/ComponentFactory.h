#ifndef UQUAD_MEDIA_COMPONENT_FACTORY_H
#define UQUAD_MEDIA_COMPONENT_FACTORY_H

#include "Config.h"
#include "../base/RefCounted.h"
#include "Component.h"
#include "SampleType.h"
#include "ContainerFormat.h"

namespace uquad
{
namespace media
{
    class VideoEncoder;
    class VideoDecoder;
    class Reader;
    class Writer;
    
    class ComponentFactory
    {
	public:
        
        typedef mi::multi_index_container
		<
			intrusive_ptr<Component::Registry>,
			mi::indexed_by
			<
				mi::random_access<>,
				mi::hashed_unique< mi::identity< intrusive_ptr<Component::Registry> > >,
				mi::ordered_non_unique
				<
					mi::member
					<
						Component::Registry,
						std::string,
						&Component::Registry::name
					>
				>,
                mi::ordered_non_unique
				<
					mi::member
					<
						Component::Registry,
						Component::Type,
						&Component::Registry::type
					>
				>
			>
		> registries_t;

		typedef mi::nth_index<registries_t, 1>::type registries_by_identity_t;
		typedef mi::nth_index<registries_t, 2>::type registries_by_name_t;
        typedef mi::nth_index<registries_t, 3>::type registries_by_type_t;
		
    protected:
        ComponentFactory();

    public:
        ~ComponentFactory();

        bool registerComponent(intrusive_ptr<Component::Registry> const &d);
        bool deregisterComponent(intrusive_ptr<Component::Registry> const &d);

        inline registries_t const& getComponents() const { return m_Registries; }

        std::pair<
            registries_by_name_t::const_iterator,
            registries_by_name_t::const_iterator
        > findComponentsByName(std::string const &n) const;

        std::pair<
       	   registries_by_type_t::const_iterator,
       	   registries_by_type_t::const_iterator
       > findComponentsByType(Component::Type const &type) const;

       template <typename C, typename Exp>
       inline void enumerateComponents(Exp exp) const
       {
    	   std::pair<
    	   	   registries_by_type_t::const_iterator,
    	       registries_by_type_t::const_iterator
    	   > registries = findComponentsByType(Component::Type(typeid(C)));

    	   for(registries_by_type_t::const_iterator ir = registries.first; ir != registries.second; ++ir)
    	   {
    		   if(!exp(dynamic_pointer_cast<typename C::Registry>(*ir))) break;
    	   }
       }

        template <typename C, typename Exp>
        inline void enumerateComponents(std::string const &name, Exp exp) const
        {
            std::pair<
                registries_by_name_t::const_iterator,
                registries_by_name_t::const_iterator
            > registries = findComponentsByName(name);

            Component::Type const t = typeid(C);
            for(registries_by_name_t::const_iterator ir = registries.first; ir != registries.second; ++ir)
            {
                if((*ir)->type != t) continue;
                if(!exp(dynamic_pointer_cast<typename C::Registry>(*ir))) break;
            }
        }

        template <typename Exp>
        inline void enumerateComponents(std::string const &name, Exp exp) const
        {
            std::pair<
                registries_by_name_t::const_iterator,
                registries_by_name_t::const_iterator
            > registries = findComponentsByName(name);

            for(registries_by_name_t::const_iterator ir = registries.first; ir != registries.second; ++ir)
            {
                if(!exp(*ir)) break;
            }
        }

        intrusive_ptr<VideoEncoder> createVideoEncoder(eVideoCodecType vct) const;
        intrusive_ptr<VideoDecoder> createVideoDecoder(eVideoCodecType vct) const;
        intrusive_ptr<Reader> createReader(eContainerFormat fmt) const;
        intrusive_ptr<Writer> createWriter(eContainerFormat fmt) const;
        

	private:
       mutable mutex m_Sync;
       registries_t m_Registries;
	};

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_COMPONENT_FACTORY_H
