#include    "kpd3.h"
#include    "alsn-struct.h"
#include    "hardware-signals.h"

KPD3::KPD3(QObject *parent) : Device(parent)
  , velocity(0.0)
  , v_kmh(0.0)
  , wheel_radius(1.25 / 2.0)
  , code_alsn(ALSN_WHITE)
  , checkTimer(new Timer(45.0, false))
  , no_lock(true)
{
    safety_relay.set();
    connect(checkTimer, &Timer::process, this, &KPD3::slotCheckTimer);
}

KPD3::~KPD3()
{

}

void KPD3::step(double t, double dt)
{
    checkTimer->step(t, dt);

    Device::step(t, dt);
}

void KPD3::ode_system(const state_vector_t &Y,
                      state_vector_t &dYdt,
                      double t)
{

}

void KPD3::preStep(state_vector_t &Y, double t)
{
    switch (code_alsn)
    {
    case ALSN_GREEN:
        {
            checkTimer->stop();
            no_lock = true;
            break;
        }

    case ALSN_YELLOW:
        {
            if (v_kmh <= 60)
            {
                checkTimer->stop();
            }
            else
            {
                if (!checkTimer->isStarted())
                    checkTimer->start();
            }

            no_lock = true;
            break;
        }

    case ALSN_RED_YELLOW:
        {
            if (v_kmh <= 60)
            {
                if (v_kmh >= 0.5)
                {
                    if (!checkTimer->isStarted())
                        checkTimer->start();
                }
                else
                {
                    checkTimer->stop();
                }

                no_lock = true;
            }
            else
            {
                safety_relay.reset();
                no_lock = false;
            }

            break;
        }

    case ALSN_RED:
        {
            safety_relay.reset();
            no_lock = false;
            break;
        }

    case ALSN_WHITE:
        {
            if (v_kmh <= 40)
            {
                if (!checkTimer->isStarted())
                    checkTimer->start();

                no_lock = true;
            }
            else
            {
                safety_relay.reset();
                no_lock = false;
            }

            break;
        }

    default:

        break;
    }
}

void KPD3::stepExternalControl(double t, double dt)
{
    if (control_signals.analogSignal[RB1].is_active)
    {
        if (static_cast<bool>(control_signals.analogSignal[RB1].cur_value) && no_lock)
        {
            safety_relay.set();
        }
    }
}

void KPD3::slotCheckTimer()
{
    safety_relay.reset();
}
