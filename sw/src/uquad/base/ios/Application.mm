#include "Application.h"
#import <Foundation/Foundation.h>

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

} //namespace base
} //namespace uquad