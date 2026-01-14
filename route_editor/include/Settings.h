#ifndef EDITOR_SETTINGS_H
#define EDITOR_SETTINGS_H

#include <vsg/ui/KeyEvent.h>

#include <string>

struct settings_t
{
    std::string window_title = "Route Editor";    ///< Window title
    int window_x = 50;                            ///< Window horizontal position
    int window_y = 50;                            ///< Window vertical position
    int window_width = 1280;                      ///< Window width
    int window_height = 720;                      ///< Window height
    int screen_number = 0;                        ///< Screen number
    bool fullscreen = false;                      ///< Fullscreen flag
    bool vsync = true;                            ///< Vertical sync flag

    bool double_buffer = true;                    ///< Double buffering flag
    int samples = 1;                              ///< Number of antialiasing samples

    double zNear = 0.1;                           ///< Near clip plane
    double view_distance = 2000;                  ///< View distance
    double fovy = 64.0;                           ///< Vertical view angle
    double fovy_min = 2.0;                        ///< Vertical view angle min
    double fovy_max = 100.0;                      ///< Vertical view angle max
    double pitch_min = -70.0;                     ///< Vertical angle down max
    double pitch_max = 70.0;                      ///< Vertical angle up max

    double gizmo_arrow_length = 5.0;
    double gizmo_arrow_thickness = 0.1;
    double gizmo_x_axis_color[3] = {1.0, 0.0, 0.0};
    double gizmo_y_axis_color[3] = {0.0, 1.0, 0.0};
    double gizmo_z_axis_color[3] = {0.0, 0.0, 1.0};
    double gizmo_opacity = 1.0;

    double gui_font_size = 20.0;
    bool is_gui_editable = false;
    bool show_objects_ref = true;
    bool show_route_map = false;
    bool show_controls = true;
    bool show_camera_settings = false;
    bool show_topology = false;

    double min_camera_rotate_speed = 10.0;
    double max_camera_rotate_speed = 25.0;
    double camera_rotate_speed = 10.0;

    double min_camera_zoom_power = 50.0;
    double max_camera_zoom_power = 200.0;
    double camera_zoom_power = 100.0;

    double min_camera_move_speed = 50.0;
    double max_camera_move_speed = 200.0;
    double camera_move_speed = 100.0;

    vsg::KeySymbol key_move_camera_forward = vsg::KEY_q;
    vsg::KeySymbol key_move_camera_backward = vsg::KEY_q;
    vsg::KeySymbol key_move_camera_left = vsg::KEY_q;
    vsg::KeySymbol key_move_camera_right = vsg::KEY_q;

    void read(const std::string& cfg_path);
};

#endif // EDITOR_SETTINGS_H
