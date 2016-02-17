#ifndef UQUAD_BASE_ERROR_H
#define UQUAD_BASE_ERROR_H

#include "Config.h"

namespace uquad
{
namespace base
{
	enum Error
	{
		kENoError = 0,
        kENotFound,
        kEAlreadyExists,
        kEOutOfRange,
        kEInvalidState,
        kEInvalidArgument,
        kEInvalidContent,
        kENotApplicable,
        kEExceptionOccurred,
        kETimeOut,
        kEWouldBlock,
        kEOperationAborted,
        kEDeviceBusy,
        kENoSuchDevice,
        kENotOpened,
		kEAlreadyOpened,
        kENotStarted,
        kEAlreadyStarted
	};
    
} //namespace base
} //namespace uquad

namespace boost
{
namespace system
{
	template<>
	struct is_error_code_enum<uquad::base::Error>
	{ static const bool value = true; };

} //namespace system
} //namespace boost


namespace uquad
{
namespace base
{
	extern system::error_category const& getErrorCategory();

	inline system::error_code makeErrorCode(Error e)
	{
		return system::error_code(e, getErrorCategory());
	}

} //namespace base
} //namespace uquad




#endif //UQUAD_BASE_ERROR_H
