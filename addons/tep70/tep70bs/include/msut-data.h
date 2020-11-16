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

    bool button_start_state;

    bool button_stop_state;

    msut_input_t()
     : velocity(0.0)
     , is_KP1_KP6_on(false)
     , bc_pressure(0.0)
     , button_start_state(false)
     , button_stop_state(false)
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

    bool    is_KTN_on;

    msut_output_t()
        : acceleration(0.0)
        , mode(0)
        , is_KTN_on(false)
    {

    }
};

#endif // MSUT_DATA_H
