#ifndef UQUAD_CONTROL_CLOCK_H
#define UQUAD_CONTROL_CLOCK_H

#include "Config.h"
#include "Block.h"
#include "Signal.h"

namespace uquad
{
namespace control
{
    class Clock
        : public virtual Block
    {
    public:
        typedef float Time;
        typedef float DT;
        
        static std::string const& timeName();
        static std::string const& dTName();
        
        Clock();
        ~Clock();
        
        //bool isValid() const;
        
        //void setTimeStamp(base::TimePoint ts);
        
    protected:
        //system::error_code prepare(asio::yield_context yctx);
        
    protected:
        DefaultOutputSignal<Time> m_Time;
        DefaultOutputSignal<DT> m_DT;
        
        //base::TimePoint m_FirstTimeStamp;
        //base::TimePoint m_LastTimeStamp;
    };
    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_CLOCK_H
