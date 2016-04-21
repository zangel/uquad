#include "Signal.h"

namespace uquad
{
namespace control
{
    Signal::Signal()
    {
    }
    
    Signal::~Signal()
    {
    }
    
    InputSignal::InputSignal()
        : Signal()
        , base::Graph::InputPort()
    {
    }
    
    InputSignal::~InputSignal()
    {
    }
    
    OutputSignal::OutputSignal()
        : Signal()
        , base::Graph::OutputPort()
    {
    }
    
    OutputSignal::~OutputSignal()
    {
    }
    
    
} //namespace control
} //namespace uquad