#ifndef WINDOW_HANDLER_H
#define WINDOW_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

struct EditorContext;

class WindowHandler : public vsg::Inherit<vsg::Visitor, WindowHandler>
{
public:
    explicit WindowHandler(EditorContext& context);
};

#endif // WINDOW_HANDLER_H
