#include "Application.h"
#include <uquad/base/Error.h>

namespace uquadrc
{
namespace remote
{

    Application::Application()
        : base::Application()
    {
    }
    
    Application::~Application()
    {
    }
    
    
    void Application::buildOptions(po::options_description &desc)
    {
        desc.add_options();
    }
    
    system::error_code Application::prepare()
    {
        return base::makeErrorCode(base::kENoError);
    }
    
    void Application::cleanup()
    {
    }
    

    
} //namespace remote
} //namespace uquadrc
