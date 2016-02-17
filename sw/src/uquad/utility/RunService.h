#ifndef UQUAD_UTILITY_RUN_SERVICE_H
#define UQUAD_UTILITY_RUN_SERVICE_H

#include "Config.h"

namespace uquad
{
namespace utility
{
    template <class Rep, class Period>
    inline bool runServiceFor(asio::io_service &ios, chrono::duration<Rep,Period> const &period)
    {
        try
        {
            const chrono::system_clock::time_point start = chrono::system_clock::now();
            while(ios.run_one())
            {
                if(chrono::duration_cast< chrono::duration<Rep,Period> >(chrono::system_clock::now() - start) >= period)
                    break;
            }
        }
        catch(...)
        {
            return false;
        }
        return true;
    }

} //namespace utility
} //namespace uquad

#endif //UQUAD_UTILITY_RUN_SERVICE_H
