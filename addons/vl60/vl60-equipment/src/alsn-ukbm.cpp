#include    <alsn-ukbm.h>
#include    <../../libJournal/include/Journal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SafetyDevice::SafetyDevice(QObject *parent) : Device(parent)
  , code_alsn(0)
  , old_code_alsn(0)
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

    if (code_alsn < old_code_alsn)
    {
        Journal::instance()->info("Change signal to less allow");
        epk_state.reset();
    }

    // Включаем ламбы на ЛС в соответствии с кодом АЛСН
    alsn_process(code_alsn);

    if (is_lamp_on(RED_LAMP))
    {
        is_red.set();
        epk_state.reset();
        Journal::instance()->info("RED locomotive signal. Autostop is RESET");
        return;
    }

    if (is_lamp_on(WHITE_LAMP))
    {
        if (v_kmh > 40.0)
        {
            epk_state.reset();
            Journal::instance()->info("WHITE locomotive signal. Velocity limit");
            return;
        }

        if (!safety_timer->isStarted())
        {
            safety_timer->start();
            Journal::instance()->info("WHITE locomotive signal. Safety timer ON");
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
void SafetyDevice::code_to_log()
{
    size_t lamp_idx = 0;

    for (size_t i = 0; i < lamps.size(); i++)
    {
        if (is_lamp_on(i))
        {
            lamp_idx = i;

            switch (i)
            {
            case RED_LAMP:
                break;
            case RED_YELLOW_LAMP:
                break;
            case YELLOW_LAMP:
                break;
            case GREEN_LAMP:
                break;
            case WHITE_LAMP:
                break;
            }

            break;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SafetyDevice::onSafetyTimer()
{
    epk_state.reset();
}
