#include    <alsn-ukbm.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SafetyDevice::SafetyDevice(QObject *parent) : Device(parent)
{
    epk_state.reset();    
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

    if (code_alsn < old_code_alsn)
    {
        epk_state.reset();
    }

    // Включаем ламбы на ЛС в соответствии с кодом АЛСН
    alsn_process(code_alsn);

    if (is_lamp_on(RED_LAMP))
    {
        is_red.set();
        epk_state.reset();

        return;
    }

    if (is_lamp_on(WHITE_LAMP))
    {
        if (v_kmh > 40.0)
        {
            epk_state.reset();            
            return;
        }

        if (!safety_timer->isStarted())
        {
            safety_timer->start();
        }
    }

    if (is_lamp_on(RED_YELLOW_LAMP))
    {
        if (v_kmh > 60.0)
        {
            epk_state.reset();
            return;
        }

        if (!safety_timer->isStarted())
        {
            safety_timer->start();
        }        
    }

    if (is_lamp_on(YELLOW_LAMP))
    {
        if (v_kmh > 60.0)
        {
            if (!safety_timer->isStarted())
                safety_timer->start();
        }
    }

    if (is_lamp_on(GREEN_LAMP))
    {
        safety_timer->stop();
    }

    if (v_kmh <= 5.0)
    {
        safety_timer->stop();
    }

    if (state_RB || state_RBS)
    {
        epk_state.set();        
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
    case ALSN_NOCODE:
        {
            if (is_lamp_on(RED_YELLOW_LAMP))
            {
                lamp_on(RED_LAMP);
            }
            else
            {
                if (!is_red.getState())
                    lamp_on(WHITE_LAMP);
            }

            break;
        }
    case ALSN_RED_YELLOW:
        {
            lamp_on(RED_YELLOW_LAMP);

            break;
        }

    case ALSN_YELLOW:
        {
            lamp_on(YELLOW_LAMP);

            break;
        }

    case ALSN_GREEN:
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
    std::fill(lamps.begin(), lamps.end(), false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SafetyDevice::lamp_on(size_t lamp_idx)
{
    off_all_lamps();
    lamps[lamp_idx] = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SafetyDevice::is_lamp_on(size_t lamp_idx)
{
    return lamps[lamp_idx];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SafetyDevice::onSafetyTimer()
{
    epk_state.reset();
}
