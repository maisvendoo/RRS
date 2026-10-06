#ifndef EDITOR_WINDOW_HANDLER_H
#define EDITOR_WINDOW_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>

struct EditorContext;

namespace vsg
{

class ConfigureWindowEvent;
class Perspective;

}

class WindowHandler : public vsg::Inherit<vsg::Visitor, WindowHandler>
{
public:
    WindowHandler(EditorContext& editor_context);

    virtual void apply(vsg::ConfigureWindowEvent& configureWindow) override;

private:
    EditorContext& editor_context;
};

#endif // EDITOR_WINDOW_HANDLER_H
