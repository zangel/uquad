#ifndef UQUAD_BASE_OBJECT_H
#define UQUAD_BASE_OBJECT_H

#include "Config.h"
#include "RefCounted.h"
#include "Runloop.h"


namespace uquad
{
namespace base
{
    class Object
    	: public RefCounted
    {
    public:
        typedef std::type_index Type;
        
        class Registry
            : public RefCounted
        {
        public:
            Registry(std::string const &n, Type const &t);
            ~Registry();
            
            virtual intrusive_ptr<Object> createObject() const = 0;
            
            template <typename T>
            inline intrusive_ptr<T> createObject() const { return dynamic_pointer_cast<T>(createObject()); }
            
            std::string name;
            Type type;
        };
        
    	Object();
    	~Object();
        
        virtual bool isValid() const = 0;

    };

} //namespace base
} //namespace uquad

#endif //UQUAD_BASE_OBJECT_H
