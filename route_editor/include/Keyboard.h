#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <bitset>
#include <cstdint>

namespace vsg
{

class KeyPressEvent;
class KeyReleaseEvent;

}

class Keyboard
{
public:
    void handle_key_press(vsg::KeyPressEvent& keyPress);
    void handle_key_release(vsg::KeyReleaseEvent& keyRelease);

private:
    std::bitset<UINT16_MAX + 1> key_states;
};

#endif // KEYBOARD_H
