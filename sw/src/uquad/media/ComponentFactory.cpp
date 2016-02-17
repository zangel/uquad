#include "ComponentFactory.h"
#include "VideoEncoder.h"
#include "VideoDecoder.h"
#include "Reader.h"
#include "Writer.h"

namespace uquad
{
namespace media
{
    ComponentFactory::ComponentFactory()
    	: m_Sync()
    	, m_Registries()
	{
        Component::registerComponents<VideoEncoder>(*this);
        Component::registerComponents<VideoDecoder>(*this);
        Component::registerComponents<Reader>(*this);
        Component::registerComponents<Writer>(*this);
	}

	ComponentFactory::~ComponentFactory()
	{

	}
    
    bool ComponentFactory::registerComponent(intrusive_ptr<Component::Registry> const &r)
    {
        lock_guard<mutex> lock(m_Sync);
		return m_Registries.push_back(r).second;
    }
    
    bool ComponentFactory::deregisterComponent(intrusive_ptr<Component::Registry> const &r)
    {
        lock_guard<mutex> lock(m_Sync);
		registries_by_identity_t &registries_by_identity = m_Registries.get<1>();
		registries_by_identity_t::iterator itr = registries_by_identity.find(r);
		if(itr == registries_by_identity.end())
		{
			return false;
		}
		registries_by_identity.erase(itr);
		return true;
    }
    
    std::pair<
    	ComponentFactory::registries_by_name_t::const_iterator,
    	ComponentFactory::registries_by_name_t::const_iterator
	> ComponentFactory::findComponentsByName(std::string const &n) const
    {
    	registries_by_name_t const &registries_by_name = m_Registries.get<2>();
    	return registries_by_name.equal_range(n);
    }
    

    std::pair<
        ComponentFactory::registries_by_type_t::const_iterator,
        ComponentFactory::registries_by_type_t::const_iterator
    > ComponentFactory::findComponentsByType(Component::Type const &type) const
    {
        registries_by_type_t const &registries_by_type = m_Registries.get<3>();
		return registries_by_type.equal_range(type);
    }
    
    intrusive_ptr<VideoEncoder> ComponentFactory::createVideoEncoder(eVideoCodecType vct) const
    {
        intrusive_ptr<media::VideoEncoder::Registry> match;
        enumerateComponents<VideoEncoder>([&match, vct](intrusive_ptr<media::VideoEncoder::Registry> const &registry) -> bool
        {
            if(registry->getVideoCodecType() == vct)
            {
                match = registry;
                return false;
            }
            return true;
        });
        
        if(match)
            return match->createComponent<VideoEncoder>();
        
        return intrusive_ptr<VideoEncoder>();
    }
    
    intrusive_ptr<VideoDecoder> ComponentFactory::createVideoDecoder(eVideoCodecType vct) const
    {
        intrusive_ptr<media::VideoDecoder::Registry> match;
        enumerateComponents<VideoDecoder>([&match, vct](intrusive_ptr<media::VideoDecoder::Registry> const &registry) -> bool
        {
            if(registry->getVideoCodecType() == vct)
            {
                match = registry;
                return false;
            }
            return true;
        });
        
        if(match)
            return match->createComponent<VideoDecoder>();
        
        return intrusive_ptr<VideoDecoder>();
    }
    
    intrusive_ptr<Reader> ComponentFactory::createReader(eContainerFormat fmt) const
    {
        intrusive_ptr<media::Reader::Registry> match;
        enumerateComponents<Reader>([&match, fmt](intrusive_ptr<media::Reader::Registry> const &registry) -> bool
        {
            if(registry->getContainerFormat() == fmt)
            {
                match = registry;
                return false;
            }
            return true;
        });
        
        if(match)
            return match->createComponent<Reader>();
        
        return intrusive_ptr<Reader>();
    }
    
    
    intrusive_ptr<Writer> ComponentFactory::createWriter(eContainerFormat fmt) const
    {
        intrusive_ptr<media::Writer::Registry> match;
        enumerateComponents<Writer>([&match, fmt](intrusive_ptr<media::Writer::Registry> const &registry) -> bool
        {
            if(registry->getContainerFormat() == fmt)
            {
                match = registry;
                return false;
            }
            return true;
        });
        
        if(match)
            return match->createComponent<Writer>();
        
        return intrusive_ptr<Writer>();
    }


} //namespace media
} //namespace uquad
