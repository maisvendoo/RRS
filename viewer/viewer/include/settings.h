//------------------------------------------------------------------------------
//
//      Viewer settings structure (from settings.xml file)
//      (c) maisvendoo
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Viewer settings structure (from settings.xml file)
 * \copyright maisvendoo
 * \author maisvendoo
 * \date
 */

#ifndef     SETTINGS_H
#define     SETTINGS_H

#include    <string>
#include    <osg/Vec3>

#include    "tcp-client.h"

/*!
 * \struct
 * \brief Main viewer settings
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct settings_t
{
    /// TCP-Client settings
    tcp_config_t    tcp_config;
    /// Interval for vehicles positions update, ms
    int vehicles_pos_update_interval;
    /// Interval for vehicles states update, ms
    int vehicles_state_update_interval;
    /// Route directory name
    std::string     route_dir_name;
    /// Route directory
    std::string     route_dir_full_path; // Temporary for displays with route map
    /// Window horizontal position
    int             x;
    /// Window vertical position
    int             y;
    /// Window width
    int             width;
    /// Window height
    int             height;
    /// Fullscreen flag
    bool            fullscreen;
    ///
    bool            vsync;
    /// Vertical view angle
    double          fovy;
    /// Vertical view angle min
    double          fovy_min;
    /// Vertical view angle max
    double          fovy_max;
    ///
    double          zNear;
    ///
    double          zFar;
    /// Screen number
    unsigned int    screen_number;
    /// Client name for server autorization
    std::string     name;
    /// Set/unset window decorations
    bool            window_decoration;
    /// Set/unset double buffering
    bool            double_buffer;
    /// Set number of anialiasing samples
    bool            samples;
    /// Motion blur persistence
    double          persistence;
    /// Cabine driver's eye height
    float           eye_height;
    /// Route motion direction
    int             direction;
    /// Notify level
    std::string     notify_level;
    /// View distance
    float           view_distance;

    /// Initial distance of extrnal camera
    float           ext_cam_init_dist;
    /// Initial height of extarnel camera
    float           ext_cam_init_height;
    /// Inital shift of external camera
    float           ext_cam_init_shift;
    /// External camera rotation coefficient
    float           ext_cam_rot_coeff;
    /// External camera motion speed
    float           ext_cam_speed;
    /// External camera speed coeff
    float           ext_cam_speed_coeff;
    /// External camera minimal distance
    float           ext_cam_min_dist;
    /// External camera initial horizontal angle
    float           ext_cam_init_angle_H;
    /// External camera initial vertical angle
    float           ext_cam_init_angle_V;

    /// Free camera initial position
    osg::Vec3       free_cam_init_pos;
    /// Free camera rotation coeff
    float           free_cam_rot_coeff;
    /// Free camera speed
    float           free_cam_speed;
    /// Free camera speed coeff
    float           free_cam_speed_coeff;
    /// Free camera FovY step
    double          free_cam_fovy_step;

    /// Cabine camera rotation coeff
    float           cabine_cam_rot_coeff;
    /// Cabine camera FovY step
    double          cabine_cam_fovy_step;
    /// Cabine camera speed
    float           cabine_cam_speed;

    /// Cabine camera relative vertical shift
    float           cabine_cam_z_min;
    /// Cabine camera relative vertical shift
    float           cabine_cam_z_max;

    /// Static camera shift
    float           stat_cam_dist;
    /// Static camera height
    float           stat_cam_height;
    /// Static camera shift
    float           stat_cam_shift;

    unsigned int    interval;

    settings_t()
        : vehicles_pos_update_interval(70)
        , vehicles_state_update_interval(100)
        , route_dir_name("")
        , route_dir_full_path("")
        , x(50)
        , y(50)
        , width(1280)
        , height(720)
        , fullscreen(false)
        , vsync(true)
        , fovy(30.0)
        , fovy_min(2.0)
        , fovy_max(120.0)
        , zNear(1.0)
        , zFar(1000.0)
        , screen_number(0)
        , name("viewer")
        , window_decoration(true)
        , double_buffer(true)
        , samples(4)
        , persistence(0.05)
        , eye_height(3.0)
        , direction(1)
        , notify_level("INFO")
        , view_distance(1000.0f)
        , ext_cam_init_dist(25.0f)
        , ext_cam_init_height(3.0f)
        , ext_cam_init_shift(0.0f)
        , ext_cam_rot_coeff(1.0f)
        , ext_cam_speed(5.0f)
        , ext_cam_speed_coeff(10.0f)
        , ext_cam_min_dist(5.0f)
        , ext_cam_init_angle_H(0.0f)
        , ext_cam_init_angle_V(0.0f)
        , free_cam_init_pos(osg::Vec3(2.5, 0.0, 1.75))
        , free_cam_rot_coeff(1.0f)
        , free_cam_speed(5.0f)
        , free_cam_speed_coeff(10.0f)
        , free_cam_fovy_step(1.0)
        , cabine_cam_rot_coeff(1.0f)
        , cabine_cam_fovy_step(1.0)
        , cabine_cam_speed(5.0f)
        , cabine_cam_z_min(-1.0f)
        , cabine_cam_z_max(0.5f)
        , stat_cam_dist(8.0f)
        , stat_cam_height(1.75)
        , stat_cam_shift(150.0f)
        , interval(10)

    {

    }
};

#endif // SETTINGS_H
