#ifndef UQUAD_UTILITY_SINGLETON_H
#define UQUAD_UTILITY_SINGLETON_H

#include "Config.h"

namespace uquad
{
namespace utility
{
    template <typename T>
    class Singleton
        : protected T
    {
    public:
    
        static T& instance()
        {
            static Singleton<T> _instance;
            return _instance;
        }
    };
        
} //namespace utility
} //namespace uquad

#define SI(clazz) uquad::utility::Singleton<clazz>::instance()

#endif //UQUAD_UTILITY_SINGLETON_H
