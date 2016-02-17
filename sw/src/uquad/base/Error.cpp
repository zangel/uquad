#include "Error.h"

namespace uquad
{
namespace base
{
	namespace
	{
		class ErrorCategory
			: public system::error_category
		{
		public:
			ErrorCategory()
				: system::error_category()
			{
			}

			const char* name() const BOOST_SYSTEM_NOEXCEPT
			{
				return "base";
			}

			system::error_condition default_error_condition(int ev) const BOOST_SYSTEM_NOEXCEPT
			{
				static system::error_category const& errCat = getErrorCategory();
				return system::error_condition(ev, errCat);
			}

			std::string message(int ev) const
			{
				return std::string("base error");
			}
		};
	}

    const system::error_category& getErrorCategory()
    {
      static ErrorCategory const ecat;
      return ecat;
    }

} //namespace hal
} //namespace uquad
