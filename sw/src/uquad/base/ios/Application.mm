#include "Application.h"

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface UQuadApplicationDelegate : UIResponder <UIApplicationDelegate>
{
    uquad::thread *m_RunloopThread;
}
@end

namespace uquad
{
namespace base
{
    std::string Application::name()
    {
        NSData* processNameDataBytes = [[[NSProcessInfo processInfo] processName] dataUsingEncoding : NSASCIIStringEncoding];
        return std::string(
            reinterpret_cast<std::string::value_type const *>([processNameDataBytes bytes]),
            [processNameDataBytes length] / sizeof(std::string::value_type)
        );
    }
    
    fs::path Application::dataDirectory()
    {
        NSString *dataDirPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
        
        NSData* dataDirPathBytes = [dataDirPath dataUsingEncoding : NSUTF8StringEncoding];
        return std::string(
            reinterpret_cast<std::string::value_type const *>([dataDirPathBytes bytes]),
            [dataDirPathBytes length] / sizeof(std::string::value_type)
        );
    }
    
    system::error_code Application::spin()
    {
        return system::error_code(
            UIApplicationMain(0, 0, nil, NSStringFromClass([UQuadApplicationDelegate class])),
            system::posix_category
        );
    }

} //namespace base
} //namespace uquad


@implementation UQuadApplicationDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    m_RunloopThread = new uquad::thread([]()
    {
        uquad::base::Application::instance()->mainRunloop()->run();
    });
    
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
}

- (void)applicationWillTerminate:(UIApplication *)application
{
}

@end