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
    float pitch_min;        ///< Vertical angle down max
    float pitch_max;        ///< Vertical angle up max

    double camera_initial_height = 0.0;

    double camera_move_speed = 100.0;
    double min_camera_move_speed = 50.0;
    double max_camera_move_speed = 200.0;

    double camera_rotate_speed = 10.0;
    double min_camera_rotate_speed = 10.0;
    double max_camera_rotate_speed = 25.0;

    double camera_zoom_power = 100.0;
    double min_camera_zoom_power = 50.0;
    double max_camera_zoom_power = 200.0;

    float gizmo_arrow_length = 5.0f;
    float gizmo_arrow_thickness = 0.1f;
    vsg::vec3 gizmo_arrow_x_color = {1.0f, 0.0f, 0.0f};
    vsg::vec3 gizmo_arrow_y_color = {0.0f, 1.0f, 0.0f};
    vsg::vec3 gizmo_arrow_z_color = {0.0f, 0.0f, 1.0f};
    float gizmo_opacity = 1.0f;
    bool gizmo_to_center = false;

    double gui_font_size = 20.0;
    bool is_gui_editable = false;
    bool show_objects_ref = true;
    bool show_route_map = false;
    bool show_controls = true;
    bool show_camera_settings = false;
    bool show_topology = false;

    KeyBindings key_bindings;
};

#endif // EDITOR_SETTINGS_H
