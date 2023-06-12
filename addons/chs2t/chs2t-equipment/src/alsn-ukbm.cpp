#include    "alsn-ukbm.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SafetyDevice::SafetyDevice(QObject *parent) : Device(parent)
  , code_alsn(1)
  , old_code_alsn(1)
  , state_RB(false)
  , state_RBS(false)
  , state_EPK(false)
  , v_kmh(0.0)
  , key_epk(false)
{
    epk_state.reset();

    safety_timer = new Timer(45.0, false);
    connect(safety_timer, &Timer::process, this, &SafetyDevice::onSafetyTimer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SafetyDevice::~SafetyDevice()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SafetyDevice::step(double t, double dt)
{
    safety_timer->step(t, dt);
    Device::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SafetyDevice::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    // Ничего не делаем при выключенном ЭПК
    if (!key_epk)
    {
        off_all_lamps();
        is_red.reset();
        return;
    }

    if (is_red.getState())
        return;

    if (code_alsn < old_code_alsn)
        epk_state.reset();

    if (code_alsn == 0)
    {
        if (old_code_alsn == 1)
        {
            is_red.set();
            epk_state.reset();
            lamp_on(RED_LAMP);
            return;
        }
        else
        {
            lamp_on(WHITE_LAMP);
        }
    }

    alsn_process(code_alsn);

    if (code_alsn == 0)
    {
        if (v_kmh > 40.0)
        {
            epk_state.reset();
            return;
        }

        if ( (!safety_timer->isStarted()) && (v_kmh > 5) )
        {
            safety_timer->start();
        }
        else
        {
            safety_timer->stop();
        }
    }

    if (code_alsn == 1)
    {
        if (v_kmh > 60.0)
        {
            epk_state.reset();
            return;
        }

        if ( (!safety_timer->isStarted()) && (v_kmh > 5) )
        {
            safety_timer->start();
        }
        else
        {
            safety_timer->stop();
        }
    }

    if (code_alsn == 2)
    {
        if (v_kmh > 60.0)
        {
            if (!safety_timer->isStarted())
                safety_timer->start();
        }
        else
        {
            safety_timer->stop();
        }
    }

    if (state_RB || state_RBS)
    {
        epk_state.set();
        safety_timer->stop();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SafetyDevice::ode_system(const state_vector_t &Y,
                              state_vector_t &dYdt, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SafetyDevice::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SafetyDevice::alsn_process(int code_alsn)
{
    switch (code_alsn)
    {
    case 1:
        {
            lamp_on(RED_YELLOW_LAMP);

            break;
        }

    case 2:
        {
            lamp_on(YELLOW_LAMP);

            break;
        }

    case 3:
        {
            lamp_on(GREEN_LAMP);

            break;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SafetyDevice::off_all_lamps()
{
    // Очищаем состояние ламп
    std::fill(lamps.begin(), lamps.end(), 0.0f);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SafetyDevice::lamp_on(size_t lamp_idx)
{
    off_all_lamps();
    lamps[lamp_idx] = 1.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SafetyDevice::onSafetyTimer()
{
    epk_state.reset();
}
