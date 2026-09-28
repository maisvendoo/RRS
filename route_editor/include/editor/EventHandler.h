#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

struct EditorContext;

namespace vsg
{

class ButtonPressEvent;
class KeyPressEvent;
class KeyReleaseEvent;
class MoveEvent;
class ScrollWheelEvent;

}

class EventHandler : public vsg::Inherit<vsg::Visitor, EventHandler>
{
public:
    explicit EventHandler(EditorContext& editor_context);

    virtual ~EventHandler() override;

    virtual void apply(vsg::KeyPressEvent& keyPress) override;

    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;

    virtual void apply(vsg::ButtonPressEvent& buttonPress) override;

    virtual void apply(vsg::ButtonReleaseEvent& buttonRelease) override;

    virtual void apply(vsg::MoveEvent& moveEvent) override;

    virtual void apply(vsg::ScrollWheelEvent& scrollWheel) override;

    void update(double delta_time);

private:
    EditorContext& editor_context;
};

#endif // EVENT_HANDLER_H
