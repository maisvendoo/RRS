#ifndef WINDOW_HANDLER_H
#define WINDOW_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>

struct EditorContext;

namespace vsg
{

class Window;

}

class WindowHandler : public vsg::Inherit<vsg::Visitor, WindowHandler>
{
public:
    explicit WindowHandler(const EditorContext& context);

    vsg::ref_ptr<vsg::Window> get_window() const;

private:
    vsg::ref_ptr<vsg::Window> window;
};

#endif // WINDOW_HANDLER_H
