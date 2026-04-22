#ifndef STATE_H
#define STATE_H

namespace vsg
{

class KeyPressEvent;
class KeyReleaseEvent;

}

class State
{
public:
    virtual ~State();
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress);
    virtual void handle_key_release(vsg::KeyReleaseEvent& keyRelease);
};

#endif // STATE_H
