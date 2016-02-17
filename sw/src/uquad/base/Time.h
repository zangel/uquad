#ifndef UQUAD_BASE_TIME_H
#define UQUAD_BASE_TIME_H

#include "Config.h"

namespace uquad
{
namespace base
{
    typedef chrono::high_resolution_clock TimeClock;
    typedef TimeClock::time_point TimePoint;
    typedef TimeClock::duration TimeDuration;
    
} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_TIME_H
