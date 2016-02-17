#include "ObjectFactory.h"
#include "../base/Error.h"

namespace uquad
{
namespace base
{
    ObjectFactory::ObjectFactory()
    	: m_Guard()
    	, m_Registries()
	{
	}

	ObjectFactory::~ObjectFactory()
	{

	}
    
    bool ObjectFactory::registerObject(intrusive_ptr<Object::Registry> const &oreg)
    {
        lock_guard<mutex> guard(m_Guard);
		return m_Registries.push_back(oreg).second;
    }
    
    bool ObjectFactory::deregisterObject(intrusive_ptr<Object::Registry> const &oreg)
    {
        lock_guard<mutex> guard(m_Guard);
		registries_by_identity_t &registries_by_identity = m_Registries.get<1>();
		registries_by_identity_t::iterator itr = registries_by_identity.find(oreg);
		if(itr == registries_by_identity.end())
		{
			return false;
		}
		registries_by_identity.erase(itr);
		return true;
    }
    
    std::pair<
    	ObjectFactory::registries_by_name_t::const_iterator,
    	ObjectFactory::registries_by_name_t::const_iterator
	> ObjectFactory::findRegistriesByName(std::string const &n) const
    {
    	registries_by_name_t const &registries_by_name = m_Registries.get<2>();
    	return registries_by_name.equal_range(n);
    }
    

    std::pair<
        ObjectFactory::registries_by_type_t::const_iterator,
        ObjectFactory::registries_by_type_t::const_iterator
    > ObjectFactory::findRegistriesByType(Object::Type const &type) const
    {
        registries_by_type_t const &registries_by_type = m_Registries.get<3>();
		return registries_by_type.equal_range(type);
    }
  
} //namespace base
} //namespace uquad

