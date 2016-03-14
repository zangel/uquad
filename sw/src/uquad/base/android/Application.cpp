#include "Application.h"
#include <boost/filesystem/operations.hpp>

namespace uquad
{
namespace base
{
    fs::path Application::dataDirectory()
    {
        return fs::current_path();
    }
    
    system::error_code Application::spin()
    {
        return m_MainRunloop.run();
    }

} //namespace base
} //namespace uquad
