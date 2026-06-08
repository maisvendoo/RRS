#ifndef EDITOR_SETTINGS_H
#define EDITOR_SETTINGS_H

#include "KeyBinding.h"
#include "settings/CameraSettings.h"
#include "settings/GizmoSettings.h"
#include "settings/GuiSettings.h"
#include "settings/SceneSettings.h"
#include "settings/WindowSettings.h"

#include <string>

struct settings_t
{
    window_settings_t window_settings;
    scene_settings_t scene_settings;
    camera_settings_t camera_settings;
    gizmo_settings_t gizmo_settings;
    gui_settings_t gui_settings;

    KeyBindings key_bindings;

    settings_t();
    void read(const std::string& cfg_path);
};

#endif // EDITOR_SETTINGS_H
