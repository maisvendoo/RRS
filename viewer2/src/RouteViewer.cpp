#include "RouteViewer.h"

#include "filesystem.h"
#include "settings.h"

RouteViewer::RouteViewer(int argc, char* argv[])
{

}

bool RouteViewer::isReady()
{
    return true;
}

int run()
{
    return 0;
}

bool RouteViewer::init(int argc, char* argv[])
{
    FileSystem& filesystem = FileSystem::getInstance();



    return true;
}

settings_t RouteViewer::loadSettings(const std::string& cfg_path) const
{
    settings_t settings;
    return settings;
}
