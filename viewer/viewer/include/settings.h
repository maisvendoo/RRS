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

/*!
 * \struct
 * \brief Main viewer settings
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct settings_t
{
    /// Route directory
    std::string     route_dir;
    /// Train config file name
    std::string     train_config;
    /// Server ip-address
    std::string     host_addr;
    /// Server port
    int             port;
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
    /// Work in localmode (reserved)
    bool            localmode;
    /// Vertical view angle
    double          fovy;
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
    /// Set client's data request time interval
    int             request_interval;
    /// Set client reconnection interval
    int             reconnect_interval;
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
    float           free_cam_fovy_step;

    /// Cabine camera rotation coeff
    float           cabine_cam_rot_coeff;
    /// Cabine camera FovY step
    float           cabine_cam_fovy_step;
    /// Cabine camera speed
    float           cabine_cam_speed;

    /// Static camera shift
    float           stat_cam_dist;
    /// Static camera height
    float           stat_cam_height;
    /// Static camera shift
    float           stat_cam_shift;

    unsigned int    interval;

    settings_t()
        : route_dir("")
        , train_config("")
        , host_addr("127.0.0.1")
        , port(1992)
        , x(50)
        , y(50)
        , width(1366)
        , height(768)
        , fullscreen(false)
        , vsync(true)
        , localmode(false)
        , fovy(30.0)
        , zNear(1.0)
        , zFar(1000.0)
        , screen_number(0)
        , name("viewer")
        , window_decoration(true)
        , double_buffer(true)
        , samples(4)
        , request_interval(200)
        , reconnect_interval(1000)
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
        , free_cam_fovy_step(1.0f)
        , cabine_cam_rot_coeff(1.0f)
        , cabine_cam_fovy_step(1.0f)
        , cabine_cam_speed(2.0)
        , stat_cam_dist(10.0f)
        , stat_cam_height(1.75)
        , stat_cam_shift(200.0f)
        , interval(10)

    {

    }
};

#endif // SETTINGS_H
