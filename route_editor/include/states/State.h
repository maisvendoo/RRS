#ifndef EDITOR_STATE_H2
#define EDITOR_STATE_H2

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;

namespace vsg
{

class KeyPressEvent;
class KeyReleaseEvent;

}

class State
{
public:
    State(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard
    );

    virtual ~State();

    virtual void handle_key_press(vsg::KeyPressEvent& keyPress);

    virtual void handle_key_release(vsg::KeyReleaseEvent& keyRelease);

protected:
    const vsg::ref_ptr<Mouse>& mouse;
    const vsg::ref_ptr<Keyboard>& keyboard;
};

#endif // EDITOR_STATE_H2
