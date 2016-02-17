#include "Config.h"
#include "Application.h"

int main(int argc, char *argv[])
{
    return cvcc::local::Application(argc, argv).exec();
}
