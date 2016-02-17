#include "Object.h"

namespace uquad
{
namespace base
{
    Object::Registry::Registry(std::string const &n, Type const &t)
        : RefCounted()
        , name(n)
        , type(t)
    {
    }
    
    Object::Registry::~Registry()
    {
    }
    
    Object::Object()
        : RefCounted()
    {
    }
    
    Object::~Object()
    {
    }
    
} //namespace base
} //namespace uquad