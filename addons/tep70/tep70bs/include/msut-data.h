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

    bool is_KMN_on;

    bool is_KD_on;

    msut_input_t()
     : velocity(0.0)
     , is_KP1_KP6_on(false)
     , bc_pressure(0.0)
     , button_start_state(false)
     , button_stop_state(false)
     , is_KMN_on(false)
     , is_KD_on(false)

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

    int     screen_num;

    double  oil_pump_timer;

    double  starter_timer;

    double  stop_timer;

    msut_output_t()
        : acceleration(0.0)
        , mode(0)
        , is_KTN_on(false)
        , screen_num(2)
        , oil_pump_timer(60)
        , starter_timer(12)
        , stop_timer(60)
    {

    }
};

#endif // MSUT_DATA_H
