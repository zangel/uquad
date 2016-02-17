#include "Application.h"
#include "Error.h"

namespace uquad
{
namespace base
{
    namespace
    {
        static Application *g_App = 0;
    }
    
    Application::Application()
        : m_RunMutex()
        , m_MainRunloop()
        , m_OptsVariableMap()
    {
        intrusive_ptr_add_ref(&m_MainRunloop);
        g_App = this;
    }
    
    
    Application::~Application()
    {
    }
    
    Application* Application::instance()
    {
        return g_App;
    }
    
    intrusive_ptr<Runloop> Application::mainRunloop()
    {
        return intrusive_ptr<Runloop>(&m_MainRunloop);
    }
    
    int Application::run(int argc, char *argv[])
    {
        lock_guard<mutex> run_guard(m_RunMutex);
        
        m_OptsVariableMap.clear();
        po::options_description desc("Options");
        desc.add_options()
            ("help", "produce a help message");
        
        buildOptions(desc);
        po::store(po::parse_command_line(argc, argv, desc), m_OptsVariableMap);
        po::notify(m_OptsVariableMap);
        
        if(m_OptsVariableMap.count("help"))
        {
            std::cout << desc;
            return 0;
        }
        
        if(system::error_code pe = prepare())
            return pe.value();
        
        system::error_code re = m_MainRunloop.run();
        
        cleanup();
        return re.value();
    }
    
    void Application::buildOptions(po::options_description &desc)
    {
        
    }
    
    system::error_code Application::prepare()
    {
        return makeErrorCode(kENoError);
    }
    
    void Application::cleanup()
    {
    }
    
} //namespace base
} //namespace uquad