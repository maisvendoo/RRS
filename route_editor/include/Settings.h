#ifndef EDITOR_SETTINGS_H
#define EDITOR_SETTINGS_H

#include "KeyBinding.h"

#include <vsg/maths/vec3.h>

#include <string>

struct settings_t
{
    settings_t();

    void read(const std::string& cfg_path);

    std::string window_title;

    int window_x;          ///< Window horizontal position
    int window_y;          ///< Window vertical position
    int window_width;      ///< Window width
    int window_height;     ///< Window height
    int screen_number;     ///< Screen number
    bool fullscreen;       ///< Fullscreen flag
    bool vsync;            ///< Vertical sync flag
    bool double_buffer;    ///< Double buffering flag
    int samples;           ///< Number of antialiasing samples

    float zNear;            ///< Near clip plane
    float view_distance;    ///< View distance
    float fovy;             ///< Vertical view angle
    float fovy_min;         ///< Vertical view angle min
    float fovy_max;         ///< Vertical view angle max

    float camera_initial_height;
    float camera_move_speed;
    float camera_rotate_speed;
    float camera_zoom_power;

    float gizmo_arrow_length;
    float gizmo_arrow_thickness;
    vsg::vec3 gizmo_arrow_x_color;
    vsg::vec3 gizmo_arrow_y_color;
    vsg::vec3 gizmo_arrow_z_color;
    float gizmo_opacity;
    bool gizmo_to_center;

    double gui_font_size;
    bool is_gui_editable;
    bool show_objects_ref;
    bool show_route_map;
    bool show_controls;
    bool show_camera_settings;
    bool show_topology;

    KeyBindings key_bindings;
};

#endif // EDITOR_SETTINGS_H
