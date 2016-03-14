#ifndef UQUAD_BASE_APPLICATION_H
#define UQUAD_BASE_APPLICATION_H

#include "Config.h"
#include "Runloop.h"

namespace uquad
{
namespace base
{
    class Application
    {
    public:
        Application();
        virtual ~Application();
        
        static Application* instance();
        
        static std::string name();
        static fs::path dataDirectory();
        
        
        intrusive_ptr<Runloop> mainRunloop();
        
        inline po::variables_map const& options() const { return m_OptsVariableMap; }
        
        int run(int argc, char *argv[]);
        
    protected:
        system::error_code spin();
        
        virtual void buildOptions(po::options_description &desc);
        virtual system::error_code prepare();
        virtual void cleanup();
        
    protected:
        mutex m_RunMutex;
        Runloop m_MainRunloop;
        po::variables_map m_OptsVariableMap;
    };
    
} //namespace base
} //namespace uquad

#endif //UQUAD_BASE_APPLICATION_H