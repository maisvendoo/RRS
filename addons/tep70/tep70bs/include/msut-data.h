#ifndef     MSUT_DATA_H
#define     MSUT_DATA_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct msut_input_t
{
    double  velocity;

    bool    is_KP1_KP6_on;

    double  bc_pressure;

    msut_input_t()
     : velocity(0.0)
     , is_KP1_KP6_on(false)
     , bc_pressure(0.0)
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct msut_output_t
{
    double  acceleration;

    int     mode;

    msut_output_t()
        : acceleration(0.0)
        , mode(0)
    {

    }
};

#endif // MSUT_DATA_H
