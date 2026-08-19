#ifndef WINDOW_HANDLER_H
#define WINDOW_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>

struct window_settings_t;

namespace vsg
{

class Camera;
class ConfigureWindowEvent;
class Perspective;
class Window;

}

class WindowHandler : public vsg::Inherit<vsg::Visitor, WindowHandler>
{
public:
    WindowHandler(
        const window_settings_t& window_settings,
        vsg::ref_ptr<vsg::Window>& window,
        vsg::ref_ptr<vsg::Perspective>& perspective,
        vsg::ref_ptr<vsg::Camera>& camera
    );

    virtual void apply(vsg::ConfigureWindowEvent& configureWindow) override;

private:
    vsg::ref_ptr<vsg::Perspective>& perspective_;
    vsg::ref_ptr<vsg::Camera>& camera_;
};

#endif // WINDOW_HANDLER_H
