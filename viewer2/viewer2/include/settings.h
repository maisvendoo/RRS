#ifndef     SETTINGS_H
#define     SETTINGS_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct settings_t
{
    float   fovY;
    float   zNear;
    float   zFar;
    bool    double_buffer;
    int     samples;
    float   view_distance;

    settings_t()
        : fovY(55.0)
        , zNear(0.1)
        , zFar(10000.0)
        , double_buffer(true)
        , samples(16)
        , view_distance(1000)
    {

    }
};

#endif // SETTINGS_H
