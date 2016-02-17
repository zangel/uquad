#include "Algorithm.h"

namespace uquad
{
namespace cv
{
    Algorithm::Registry::Registry(std::string const &n, Type const &t)
        : base::RefCounted()
        , name(n)
        , type(t)
    {
    }
    
    Algorithm::Registry::~Registry()
    {
    }
    
    Algorithm::Algorithm()
        : base::RefCounted()
    {
    }
    
    Algorithm::~Algorithm()
    {
    }
    
} //namespace cv
} //namespace uquad