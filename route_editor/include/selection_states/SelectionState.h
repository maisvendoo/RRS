#ifndef SELECTION_STATE_H
#define SELECTION_STATE_H

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class KeyPressEvent;
class KeyReleaseEvent;
class MoveEvent;

}

class SelectionState
{
public:
    virtual ~SelectionState() = default;

    virtual void apply(vsg::ButtonPressEvent& buttonPress) {}
    virtual void apply(vsg::ButtonReleaseEvent& buttonRelease) {}
    virtual void apply(vsg::MoveEvent& moveEvent) {}
    virtual void apply(vsg::KeyPressEvent& keyPress) {}
    virtual void apply(vsg::KeyReleaseEvent& keyRelease) {}

private:
    SelectionState* prev_selection_state = nullptr;
};

#endif // SELECTION_STATE_H
