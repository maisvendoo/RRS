#ifndef ROUTE_EDITOR_H
#define ROUTE_EDITOR_H

#include "editor/settings/WindowSettings.h"

#include <vsg/core/ref_ptr.h>

namespace vsg
{

class Options;
class Window;

}

class RouteEditor
{
public:
    explicit RouteEditor(bool& success);
    ~RouteEditor();

private:
    window_settings_t window_settings;

    vsg::ref_ptr<vsg::Options> vsg_options;
    vsg::ref_ptr<vsg::Window> window;

private:
    void initialize_journal() const;
    bool read_settings();
    void create_window();
};

#endif // ROUTE_EDITOR_H
