#include    "time-relay.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TimeRelay::TimeRelay(size_t num_contacts,
                     double delay_on,
                     double delay_off,
                     QObject *parent) : Relay(num_contacts, parent)
  , Uc(0.0)
  , Rc(300)
  , start_level(0.7)
  , is_on(false)
  , timer_on(new Timer(delay_on, false))
  , timer_off(new Timer(delay_off, false))
{
    connect(timer_on, &Timer::process, this, &TimeRelay::slotTimeoutProcessOn);
    connect(timer_off, &Timer::process, this, &TimeRelay::slotTimeoutProcessOff);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TimeRelay::~TimeRelay()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TimeRelay::setTimeoutOn(double timeout)
{
    timer_on->setTimeout(timeout);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TimeRelay::setTimeoutOff(double timeout)
{
    timer_off->setTimeout(timeout);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TimeRelay::setControlVoltage(double Uc)
{
    this->Uc = Uc;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TimeRelay::getCurrent() const
{
    return getY(0) + Uc / Rc;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TimeRelay::step(double t, double dt)
{
    timer_on->step(t, dt);
    timer_off->step(t, dt);
    Relay::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TimeRelay::setVoltage(double U)
{
    this->U = U;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TimeRelay::preStep(state_vector_t &Y, double t)
{
    if (Uc >= U_nom * start_level)
    {
        timer_off->stop();
        if ( (!is_on) && (timer_on->getTimeout() > Physics::ZERO) )
        {
            timer_on->start();
        }
        else
        {
            is_on = true;
            setVoltage(Uc);
        }
    }
    else
    {
        timer_on->stop();
        if ( (is_on) && (timer_off->getTimeout() > Physics::ZERO) )
        {
            timer_off->start();
        }
        else
        {
            is_on = false;
            setVoltage(0.0);
        }
    }

    Relay::preStep(Y, t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TimeRelay::ode_system(const state_vector_t &Y,
                           state_vector_t &dYdt,
                           double t)
{
    Relay::ode_system(Y, dYdt, t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TimeRelay::load_config(CfgReader &cfg)
{
    Relay::load_config(cfg);
    start_level = level_on;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TimeRelay::slotTimeoutProcessOn()
{
    is_on = true;
    setVoltage(Uc);
    timer_on->stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TimeRelay::slotTimeoutProcessOff()
{
    is_on = false;
    setVoltage(0.0);
    timer_off->stop();
}
