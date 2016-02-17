#ifndef UQUAD_UTILITY_TYPE_INDEX_EXTRACTOR_H
#define UQUAD_UTILITY_TYPE_INDEX_EXTRACTOR_H

#include "Config.h"

namespace uquad
{
namespace utility
{
    struct TypeIndexExtractor
    {
        typedef std::type_index result_type;
        
        template <typename T>
        inline result_type operator()(T const &v) const
        {
            return result_type(typeid(T));
        }
        
        template <typename T>
        inline result_type operator()(T const *v) const
        {
            return result_type(typeid(T));
        }
        
        template <typename T>
        inline std::type_info operator()(intrusive_ptr<T> const &v) const
        {
            return std::type_index(typeid(T));
        }
    };
    
    
} //namespace utility
} //namespace uquad

#endif //UQUAD_UTILITY_TYPE_INDEX_EXTRACTOR_H