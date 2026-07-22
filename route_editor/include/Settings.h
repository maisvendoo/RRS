#ifndef EDITOR_SETTINGS_H
#define EDITOR_SETTINGS_H

#include "KeyBinding.h"

#include <string>

struct settings_t
{
    KeyBindings key_bindings;

    settings_t();
    void read(const std::string& cfg_path);
};

#endif // EDITOR_SETTINGS_H
