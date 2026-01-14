#ifndef WINDOW_WRAPPER_H
#define WINDOW_WRAPPER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>

struct settings_t;

namespace vsg
{

class ConfigureWindowEvent;
class Window;

}

class WindowHandler : public vsg::Inherit<vsg::Visitor, WindowHandler>
{
public:
    bool is_resized = false;

public:
    WindowHandler(const settings_t& settings);

    // TODO: Maybe remove with is_resized
    void apply(vsg::ConfigureWindowEvent& congfigureWindow) override;

    vsg::ref_ptr<vsg::Window> get_window() const;

private:
    vsg::ref_ptr<vsg::Window> window;
};

#endif // WINDOW_WRAPPER_H
