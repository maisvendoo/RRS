#ifndef EDITOR_SETTINGS_H
#define EDITOR_SETTINGS_H

#include "KeyBinding.h"
#include "settings/SceneSettings.h"
#include "settings/WindowSettings.h"

#include <string>

struct settings_t
{
    window_settings_t window_settings;
    scene_settings_t scene_settings;

    KeyBindings key_bindings;

    settings_t();
    void read(const std::string& cfg_path);
};

#endif // EDITOR_SETTINGS_H
