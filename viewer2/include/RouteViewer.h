#ifndef ROUTE_VIEWER_H
#define ROUTE_VIEWER_H

#include "SoundManager.h"
#include "settings.h"

#include <memory>
#include <string>

class RouteViewer
{
public:
    RouteViewer(int argc, char* argv[]);

    bool isReady() const;

    int run();

private:
    bool init(int argc, char* argv[]);

    void loadSettings(const std::string& cfg_path);

    int overrideSettingsByCommandLine(int argc, char* argv[]);

private:
    bool is_ready;

    settings_t settings;

    std::unique_ptr<SoundManager> sound_manager;
};

#endif // ROUTE_VIEWER_H
