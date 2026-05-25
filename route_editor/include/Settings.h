#ifndef EDITOR_SETTINGS_H
#define EDITOR_SETTINGS_H

#include "KeyBinding.h"
#include "settings/CameraSettings.h"
#include "settings/GizmoSettings.h"
#include "settings/SceneSettings.h"
#include "settings/WindowSettings.h"

#include <vsg/maths/vec3.h>

#include <string>

class CfgReader;

struct settings_t
{
    settings_t();

    void read(const std::string& cfg_path);

    window_settings_t window_settings;
    scene_settings_t scene_settings;
    camera_settings_t camera_settings;
    gizmo_settings_t gizmo_settings;

    float gui_font_size;
    bool is_gui_editable;
    bool show_objects_ref;
    bool show_route_map;
    bool show_stations_conf;
    bool show_waypoints_conf;
    bool show_key_bindings;
    bool show_camera_settings;
    bool show_topology;
    bool show_selected_objects_properties;
    bool show_commands;

    KeyBindings key_bindings;
};

#endif // EDITOR_SETTINGS_H
