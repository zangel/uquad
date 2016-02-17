#include "Config.h"
#include "Application.h"

int main(int argc, char *argv[])
{
    return cvtracker::local::Application(argc, argv).exec();
}
