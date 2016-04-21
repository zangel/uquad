#ifndef UQUADRC_REMOTE_APPLICATION_H
#define UQUADRC_REMOTE_APPLICATION_H

#include <uquad/Config.h>
#include <uquad/base/Application.h>

namespace uquadrc
{
namespace remote
{
    using namespace uquad;
    
    class Application
        : public base::Application
    {
    public:
        Application();
        ~Application();
        
        void buildOptions(po::options_description &desc);
        system::error_code prepare();
        void cleanup();

        
    protected:
        
        
    private:
    };
    
} //namespace remote
} //namespace uquadrc


#endif //UQUADRC_REMOTE_APPLICATION_H