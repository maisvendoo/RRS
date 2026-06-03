#include "editor/Keyboard.h"

#include <vsg/ui/KeyEvent.h>

bool Keyboard::pressed_once(vsg::KeySymbol key, bool ignore_handled_keys) const
{
    auto itr = keyState.find(key);
    if (itr == keyState.end())
    {
        return false;
    }

    const auto& keyHistory = itr->second;
    if (keyHistory.timeOfKeyRelease != keyHistory.timeOfFirstKeyPress)
    {
        return false;
    }

    if (keyHistory.timeOfLastKeyPress != keyHistory.timeOfFirstKeyPress)
    {
        return false;
    }

    if (ignore_handled_keys && keyHistory.handled)
    {
        return false;
    }

    return true;
}
