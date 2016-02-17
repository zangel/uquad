#include "Config.h"
#include "Application.h"

int main(int argc, char *argv[])
{
    return ctrlpe::local::Application(argc, argv).exec();
}
