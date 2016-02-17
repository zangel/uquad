#ifndef UQUAD_BASE_OBJECT_FACTORY_H
#define UQUAD_BASE_OBJECT_FACTORY_H

#include "Config.h"
#include "Object.h"

namespace uquad
{
namespace base
{
    class ObjectFactory
    {
	public:
        
        typedef mi::multi_index_container
        <
            intrusive_ptr<Object::Registry>,
            mi::indexed_by
            <
                mi::random_access<>,
                mi::hashed_unique< mi::identity< intrusive_ptr<Object::Registry> > >,
                mi::ordered_non_unique
                <
                    mi::member
                    <
                        Object::Registry,
                        std::string,
                        &Object::Registry::name
                    >
                >,
                mi::ordered_non_unique
                <
                    mi::member
                    <
                        Object::Registry,
                        Object::Type,
                        &Object::Registry::type
                    >
                >
            >
        > registries_t;
        
        typedef mi::nth_index<registries_t, 1>::type registries_by_identity_t;
		typedef mi::nth_index<registries_t, 2>::type registries_by_name_t;
        typedef mi::nth_index<registries_t, 3>::type registries_by_type_t;
		
    protected:
        ObjectFactory();

    public:
        ~ObjectFactory();

        bool registerObject(intrusive_ptr<Object::Registry> const &oreg);
        bool deregisterObject(intrusive_ptr<Object::Registry> const &oreg);

        inline registries_t const& getRegistries() const { return m_Registries; }

        std::pair<
            registries_by_name_t::const_iterator,
            registries_by_name_t::const_iterator
        > findRegistriesByName(std::string const &n) const;

        std::pair<
           registries_by_type_t::const_iterator,
           registries_by_type_t::const_iterator
        > findRegistriesByType(Object::Type const &type) const;

        template <typename Obj, typename Exp>
        inline void enumerateRegistries(Exp exp) const
        {
            std::pair<
               registries_by_type_t::const_iterator,
               registries_by_type_t::const_iterator
            > registries = findRegistriesByType(Object::Type(typeid(Obj)));
            
            for(registries_by_type_t::const_iterator ir = registries.first; ir != registries.second; ++ir)
            {
                if(!exp(dynamic_pointer_cast<typename Obj::Registry>(*ir))) break;
            }
        }

        template <typename Obj, typename Exp>
        inline void enumerateRegistries(std::string const &name, Exp exp) const
        {
            std::pair<
                registries_by_name_t::const_iterator,
                registries_by_name_t::const_iterator
            > registries = findRegistriesByName(name);

            Object::Type const t = typeid(Obj);
            for(registries_by_name_t::const_iterator ir = registries.first; ir != registries.second; ++ir)
            {
                if((*ir)->type != t) continue;
                if(!exp(dynamic_pointer_cast<typename Obj::Registry>(*ir))) break;
            }
        }

        template <typename Exp>
        inline void enumerateRegistries(std::string const &name, Exp exp) const
        {
            std::pair<
                registries_by_name_t::const_iterator,
                registries_by_name_t::const_iterator
            > registries = findRegistriesByName(name);

            for(registries_by_name_t::const_iterator ir = registries.first; ir != registries.second; ++ir)
            {
                if(!exp(*ir)) break;
            }
        }

	private:
       mutable mutex m_Guard;
       registries_t m_Registries;
	};
    
    namespace detail
    {
        template <typename O, typename OF> struct RegisterObject;
    } //namespace detail

} //namespace base
} //namespace uquad

#define UQUAD_BASE_REGISTER_OBJECT(O, OF)                                   \
    namespace uquad {                                                       \
    namespace base {                                                        \
    namespace detail {                                                      \
    template <>                                                             \
    struct RegisterObject<O, OF> {                                          \
        RegisterObject(){                                                   \
            SI(OF).registerObject(intrusive_ptr<Object::Registry>(          \
                new typename O::Registry()                                  \
            ));                                                             \
        }                                                                   \
        static RegisterObject<O, OF> instance;                              \
    };                                                                      \
    RegisterObject<O, OF>                                                   \
        RegisterObject<O, OF>::instance = RegisterObject<O, OF>();          \
    }}}

#endif //UQUAD_BASE_OBJECT_FACTORY_H
