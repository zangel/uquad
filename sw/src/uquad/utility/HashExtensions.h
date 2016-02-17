#ifndef UQUAD_UTILITY_HASH_EXTENSIONS_H
#define UQUAD_UTILITY_HASH_EXTENSIONS_H

#include "Config.h"

namespace Eigen
{
    template <typename T>
    inline std::size_t hash_value(Eigen::Matrix<T,2,1> const &v)
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, v[0]);
        boost::hash_combine(seed, v[1]);
        return seed;
    }
    
} //namespace Eigen

#endif //UQUAD_UTILITY_HASH_EXTENSIONS_H