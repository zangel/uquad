#ifndef UQUAD_TOOL_APPLICATION_H
#define UQUAD_TOOL_APPLICATION_H

#include "Config.h"

namespace uquad
{
namespace tool
{

    class MainWindow;

    class Application
        : public QApplication
    {
    public:
        Application(int &argc, char **argv);
        ~Application();
        
        static Application* instance();
        
        MainWindow* getMainWindow();
        
        hal::UQuadDevice& getUQuadDevice();
        
        
    
        
    private:
        MainWindow *m_pMainWindow;
        hal::UQuadDevice m_UQuadDevice;
    };

} //namespace tool
} //namespace uquad


#endif //UQUAD_TOOL_APPLICATION_H