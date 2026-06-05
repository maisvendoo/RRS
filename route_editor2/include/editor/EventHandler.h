#ifndef EDITOR_EVENT_HANDLER_H
#define EDITOR_EVENT_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>

#include <memory>

class EditorState;
class Keyboard;

namespace vsg
{

class FocusInEvent;
class FocusOutEvent;
class KeyPressEvent;
class KeyReleaseEvent;

}

class EventHandler : public vsg::Inherit<vsg::Visitor, EventHandler>
{
public:
    EventHandler();
    ~EventHandler();

    virtual void apply(vsg::KeyPressEvent& keyPress) override;
    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;
    virtual void apply(vsg::FocusInEvent& focusIn) override;
    virtual void apply(vsg::FocusOutEvent& focusOut) override;

private:
    vsg::ref_ptr<Keyboard> keyboard;
    std::unique_ptr<EditorState>* editor_state;
};

#endif // EDITOR_EVENT_HANDLER_H
