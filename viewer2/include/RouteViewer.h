#ifndef ROUTE_VIEWER_H
#define ROUTE_VIEWER_H

#include "settings.h"

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

private:
    bool is_ready;

    settings_t settings;
};

#endif // ROUTE_VIEWER_H
