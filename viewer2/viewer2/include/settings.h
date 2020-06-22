#ifndef     SETTINGS_H
#define     SETTINGS_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct settings_t
{
    int                 posX;
    int                 posY;
    int                 width;
    int                 height;
    bool                fullscreen;
    unsigned int        screen_num;
    bool                window_title;
    double              fovY;
    double              zNear;
    double              zFar;
    bool                double_buffer;
    unsigned int        samples;
    double              view_distance;

    settings_t()
        : posX(50)
        , posY(50)
        , width(1280)
        , height(720)
        , fullscreen(false)
        , screen_num(1)
        , window_title(true)
        , fovY(55.0)
        , zNear(0.1)
        , zFar(10000.0)
        , double_buffer(true)
        , samples(16)
        , view_distance(1000)
    {

    }
};

#endif // SETTINGS_H
