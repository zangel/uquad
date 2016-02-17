#ifndef UQUAD_CONTROL_SYSTEM_H
#define UQUAD_CONTROL_SYSTEM_H

#include "Config.h"

#include "../base/Object.h"
#include "../base/Delegate.h"
#include "../base/Runloop.h"


namespace uquad
{
namespace control
{
    class SystemLibrary;
    
    class System
    	: public base::Object
    {
    public:
    	System();
    	~System();
        
        virtual system::error_code prepare() = 0;
        virtual bool isPrepared() const = 0;
        virtual void unprepare() = 0;
    };

} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_SYSTEM_H
