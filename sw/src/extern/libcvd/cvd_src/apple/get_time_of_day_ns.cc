
#include "cvd/timer.h"
#include <mach/mach.h>
#include <mach/mach_time.h>

namespace CVD {

namespace
{
    static mach_timebase_info_data_t getMachTimeBase()
    {
        mach_timebase_info_data_t ret;
        mach_timebase_info(&ret);
        return ret;
    }
}
    
long long get_time_of_day_ns()
{
    static mach_timebase_info_data_t machTimeBase = getMachTimeBase();
    
    uint64_t machAbsTime = mach_absolute_time();
    return (machAbsTime * machTimeBase.numer)/machTimeBase.denom;
}

}
