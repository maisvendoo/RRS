#ifndef EDITOR_STATE_H2
#define EDITOR_STATE_H2

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

#endif // EDITOR_STATE_H2
