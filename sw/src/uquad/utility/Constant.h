#ifndef UQUAD_UTILITY_CONSTANT_H
#define UQUAD_UTILITY_CONSTANT_H

#include "Config.h"

namespace uquad
{
namespace utility
{
    template <typename T>
    struct Constant
    {
    	static T const& value() { static T const _value = T(); return _value; }
    };

} //namespace utility
} //namespace uquad

#define SI(clazz) uquad::utility::Singleton<clazz>::instance()

#endif //UQUAD_UTILITY_SINGLETON_H
