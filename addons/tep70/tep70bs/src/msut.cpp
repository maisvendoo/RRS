#include    "msut.h"

#include    "tep70-signals.h"



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MSUT::MSUT(QObject *parent) : Device(parent)
{
    accel_calc = new DerivativeCalculator(0.1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MSUT::~MSUT()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUT::step(double t, double dt)
{
    accel_calc->step(msut_input.velocity, t, dt);

    Device::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUT::preStep(state_vector_t &Y, double t)
{
    msut_output.acceleration = accel_calc->getDerivative();

    if (msut_input.button_start_state)
        fuel_pump_control.set();

    if (msut_input.button_stop_state)
        fuel_pump_control.reset();

    msut_output.is_KTN_on = fuel_pump_control.getState();

    if (msut_input.is_KP1_KP6_on || (msut_input.velocity >= 1.0 / Physics::kmh))
        msut_output.screen_num = 1;
    else
        msut_output.screen_num = 2;

    /*if (msut_output.mode == 0)
    {
        msut_output.stop_timer = 60;
        msut_output.starter_timer = 12;
        msut_output.oil_pump_timer = 60;
    }*/



    select_mode();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUT::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUT::load_config(CfgReader &cfg)
{

}

void MSUT::stepDiscrete(double t, double dt)
{
    (void) t;


    if (msut_input.is_KMN_on)
    {
        if (msut_output.is_KTN_on)
        {
            msut_output.mode = 5;
            msut_output.oil_pump_timer -= dt;
        }
        else
        {
            msut_output.mode = 7;
            msut_output.stop_timer -= dt;
        }
    }
    else
    {
        msut_output.stop_timer = 60;
        msut_output.oil_pump_timer = 60;
    }

    if (msut_input.is_KD_on)
    {
        msut_output.mode = 6;
        msut_output.starter_timer -= dt;
    }


    msut_output.oil_pump_timer = cut(msut_output.oil_pump_timer, 0.0, 60.0);
    msut_output.starter_timer = cut(msut_output.starter_timer, 0.0, 12.0);
    msut_output.stop_timer = cut(msut_output.stop_timer, 0.0, 60.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MSUT::select_mode()
{
    if (msut_input.is_KP1_KP6_on)
    {
        msut_output.mode = 1; // тяга
    }
    else
    {
        if (msut_input.velocity * 3.6 < 1.0)
        {
            msut_output.mode = 0; // стоп
        }
        else
        {
            if (msut_input.bc_pressure >= 0.1)
                msut_output.mode = 4; // замещение
            else
                msut_output.mode = 2; // выбег
        }
    }
}
