#ifndef     SETTINGS_H
#define     SETTINGS_H

#include    <string>

struct settings_t
{
    std::string     route_dir;
    std::string     host_addr;
    int             port;
    int             x;
    int             y;
    int             width;
    int             height;
    bool            fullscreen;
    bool            localmode;
    double          fovy;
    double          zNear;
    double          zFar;
    unsigned int    screen_number;
    std::string     window_title;
    bool            window_decoration;
    bool            double_buffer;
    bool            samples;

    settings_t()
        : route_dir("")
        , host_addr("127.0.0.1")
        , port(1992)
        , x(50)
        , y(50)
        , width(1366)
        , height(768)
        , fullscreen(false)
        , localmode(false)
        , fovy(30.0)
        , zNear(1.0)
        , zFar(1000.0)
        , screen_number(0)
        , window_title("Video 3D Client")
        , window_decoration(true)
        , double_buffer(true)
        , samples(4)
    {

    }
};

#endif // SETTINGS_H
