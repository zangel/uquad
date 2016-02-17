#include "Codec.h"

namespace uquad
{
namespace media
{
    Codec::Registry::Registry(std::string const &n, Component::Type const &type)
        : Component::Registry(n, type)
    {
    }
    
    Codec::Registry::~Registry()
    {
    }
    
    Codec::Codec()
        : Component()
        , Source()
        , Sink()
    {
    }
    
    Codec::~Codec()
    {
    }

} //namespace media
} //namespace uquad