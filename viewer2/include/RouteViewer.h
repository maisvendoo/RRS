#ifndef ROUTE_VIEWER_H
#define ROUTE_VIEWER_H

#include <string>

struct settings_t;

class RouteViewer
{
public:
    RouteViewer(int argc, char* argv[]);

    bool isReady();
    int run();

private:
    bool init(int argc, char* argv[]);
    settings_t loadSettings(const std::string& cfg_path) const;
};

#endif // ROUTE_VIEWER_H
