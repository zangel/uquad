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

} //namespace base
} //namespace uquad
