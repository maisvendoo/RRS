#include    "pneumo-combine-crane.h"

#include    "math-funcs.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoCombineCrane::PneumoCombineCrane(QObject *parent) : Device(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoCombineCrane::~PneumoCombineCrane()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoCombineCrane::setKeySymbolCombineCraneClockwise(std::uint16_t key_symbol)
{
    ref_state.setKeySymbolIncrease(key_symbol);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoCombineCrane::setKeyModifierCombineCraneClockwise(std::uint16_t key_modifier)
{
    ref_state.setKeyModifierIncrease(key_modifier);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoCombineCrane::setKeySymbolCombineCraneCounterclockwise(std::uint16_t key_symbol)
{
    ref_state.setKeySymbolDecrease(key_symbol);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoCombineCrane::setKeyModifierCombineCraneCounterclockwise(std::uint16_t key_modifier)
{
    ref_state.setKeyModifierDecrease(key_modifier);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoCombineCrane::setControl(std::set<uint16_t>* keys,
                                    control_signals_t* control_signals)
{
    Device::setControl(keys, control_signals);
    ref_state.setControl(pressed_keys);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoCombineCrane::setCombineCranePosition(int pos)
{
    ref_state.setPosition(pos + 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoCombineCrane::getCombineCraneHandlePosition() const
{
    return getY(HANDLE_POS);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoCombineCrane::setBPpressure(double value)
{
    pBP = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoCombineCrane::getCraneBPpressure() const
{
    return getY(PRESSURE);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoCombineCrane::setCraneBPflow(double value)
{
    Q = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoCombineCrane::getBPflow() const
{
    return QBP;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t PneumoCombineCrane::getSoundState(size_t idx) const
{
    if (idx == BP_DRAIN_FLOW_SOUND)
    {
        return emergency_flow_sound;
    }

    return ref_state.getSoundState(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float PneumoCombineCrane::getSoundSignal(size_t idx) const
{
    if (idx == BP_DRAIN_FLOW_SOUND)
    {
        return emergency_flow_sound.createSoundSignal();
    }

    return ref_state.getSoundSignal(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoCombineCrane::preStep(state_vector_t &Y, double t)
{
    (void) t;

    if (getY(HANDLE_POS) > 0.5)
    {
        emergency_flow_sound.state = 1;
        emergency_flow_sound.volume = K_sound * cbrt(abs(QBP));

        QBP = -K_emergency * pBP;
        return;
    }

    emergency_flow_sound.state = 0;
    emergency_flow_sound.volume = 0.0f;

    if (getY(HANDLE_POS) < -0.5)
    {
        QBP = 0.0;
    }
    else
    {
        QBP = Q;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoCombineCrane::ode_system(const state_vector_t &Y,
                                    state_vector_t &dYdt,
                                    double t)
{
    (void) t;

    const double ref_pos = static_cast<double>(ref_state.getPosition()) - 1.0;
    const double delta = ref_pos - Y[HANDLE_POS];
    if (abs(delta) > 0.05)
    {
        dYdt[HANDLE_POS] = sign(delta) / switch_time;
    }
    else
    {
        dYdt[HANDLE_POS] = 20.0 * delta / switch_time;
    }

    if (abs(Y[HANDLE_POS]) < 0.5)
    {
        setY(PRESSURE, pBP);
        dYdt[PRESSURE] = 0.0;
    }
    else
    {
        dYdt[PRESSURE] = QBP / V0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoCombineCrane::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    double tmp = 0.0;
    cfg.getDouble(secName, "SwitchTime", tmp);
    if (tmp > 0.1)
        switch_time = tmp;

    tmp = 0.0;
    cfg.getDouble(secName, "V0", tmp);
    if (tmp > 1e-3)
        V0 = tmp;

    cfg.getDouble(secName, "K_emergency", K_emergency);

    cfg.getDouble(secName, "K_sound", K_sound);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoCombineCrane::stepKeysControl(double t, double dt)
{
    ref_state.step(t, dt);
}
