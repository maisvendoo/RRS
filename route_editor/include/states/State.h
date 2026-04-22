#ifndef STATE_H
#define STATE_H

namespace vsg
{

class KeyPressEvent;

}

class State
{
public:
    virtual ~State();
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress);
};

#endif // STATE_H
