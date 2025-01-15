#ifndef VIEWER_SETTINGS_H
#define VIEWER_SETTINGS_H

#include "tcp-client.h"

#include <vsg/maths/vec3.h>

#include <string>

struct settings_t
{
    settings_t();

    tcp_config_t tcp_config;  ///< TCP-Client settings

    int vehicles_pos_update_interval;       ///< Interval for vehicles positions update, ms
    int vehicles_state_update_interval;     ///< Interval for vehicles states update, ms
    int vehicle_controled_update_interval;  ///< Interval for vehicle controlled debug strings update, ms
    int client_delay;                       ///< Client delay for smoothing network's delays

    std::string route_dir_name;  ///< Route directory name

    /// Route directory
    std::string route_dir_full_path;  // Temporary for displays with route map

    int x;            ///< Window horizontal position
    int y;            ///< Window vertical position
    int width;        ///< Window width
    int height;       ///< Window height
    bool fullscreen;  ///< Fullscreen flag
    bool vsync;

    double fovy;      ///< Vertical view angle
    double fovy_min;  ///< Vertical view angle min
    double fovy_max;  ///< Vertical view angle max

    double zNear;
    double zFar;

    unsigned int screen_number;  ///< Screen number

    std::string name;  ///< Client name for server autorization

    bool window_decoration;    ///< Set/unset window decorations
    bool double_buffer;        ///< Set/unset double buffering
    bool samples;              ///< Set number of anialiasing samples
    double persistence;        ///< Motion blur persistence
    float eye_height;          ///< Cabine driver's eye height
    int direction;             ///< Route motion direction
    std::string notify_level;  ///< Notify level
    float view_distance;       ///< View distance

    float ext_cam_init_dist;     ///< Initial distance of extrnal camera
    float ext_cam_init_height;   ///< Initial height of extarnel camera
    float ext_cam_init_shift;    ///< Inital shift of external camera
    float ext_cam_rot_coeff;     ///< External camera rotation coefficient
    float ext_cam_speed;         ///< External camera motion speed
    float ext_cam_speed_coeff;   ///< External camera speed coeff
    float ext_cam_min_dist;      ///< External camera minimal distance
    float ext_cam_init_angle_H;  ///< External camera initial horizontal angle
    float ext_cam_init_angle_V;  ///< External camera initial vertical angle

    vsg::vec3 free_cam_init_pos;  ///< Free camera initial position
    float free_cam_rot_coeff;     ///< Free camera rotation coeff
    float free_cam_speed;         ///< Free camera speed
    float free_cam_speed_coeff;   ///< Free camera speed coeff
    double free_cam_fovy_step;    ///< Free camera FovY step

    float cabine_cam_rot_coeff;   ///< Cabine camera rotation coeff
    double cabine_cam_fovy_step;  ///< Cabine camera FovY step
    float cabine_cam_speed;       ///< Cabine camera speed
    float cabine_cam_z_min;       ///< Cabine camera relative vertical shift
    float cabine_cam_z_max;       ///< Cabine camera relative vertical shift

    float stat_cam_dist;    ///< Static camera shift
    float stat_cam_height;  ///< Static camera height
    float stat_cam_shift;   ///< Static camera shift

    unsigned int interval;
};

#endif // VIEWER_SETTINGS_H
