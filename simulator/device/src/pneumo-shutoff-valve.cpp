#include    "pneumo-shutoff-valve.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoShutoffValve::PneumoShutoffValve(QObject *parent) : Device(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoShutoffValve::~PneumoShutoffValve()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoShutoffValve::setKeySymbolOpen(std::uint16_t key_symbol)
{
    ref_state.setKeySymbolOn(key_symbol);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoShutoffValve::setKeyModifierOpen(std::uint16_t key_modifier)
{
    ref_state.setKeyModifierOn(key_modifier);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoShutoffValve::setKeySymbolClose(std::uint16_t key_symbol)
{
    ref_state.setKeySymbolOff(key_symbol);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoShutoffValve::setKeyModifierClose(std::uint16_t key_modifier)
{
    ref_state.setKeyModifierOff(key_modifier);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoShutoffValve::setControl(std::set<uint16_t>* keys,
                                    control_signals_t* control_signals)
{
    Device::setControl(keys, control_signals);
    ref_state.setControl(pressed_keys);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoShutoffValve::close()
{
    ref_state.reset();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoShutoffValve::open()
{
    ref_state.set();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PneumoShutoffValve::isOpened() const
{
    return is_opened;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoShutoffValve::getHandlePosition() const
{
    return getY(HANDLE_POS);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoShutoffValve::setPipePressure(double value)
{
    p = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoShutoffValve::setDeviceFlow(double value)
{
    Q = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoShutoffValve::getPressureToDevice() const
{
    return getY(PRESSURE);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoShutoffValve::getFlowToPipe() const
{
    if (is_opened)
        return Q;
    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t PneumoShutoffValve::getSoundState(size_t idx) const
{
    if (idx == DRAIN_FLOW_SOUND)
        return atm_flow_sound;
    return ref_state.getSoundState(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float PneumoShutoffValve::getSoundSignal(size_t idx) const
{
    if (idx == DRAIN_FLOW_SOUND)
        return atm_flow_sound.createSoundSignal();
    return ref_state.getSoundSignal(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoShutoffValve::preStep(state_vector_t &Y, double t)
{
    (void) t;

    if (Y[HANDLE_POS] > 0.05)
    {
        is_opened = true;
        Q_atm = 0.0;
        atm_flow_sound.state = 0;
        atm_flow_sound.volume = 0.0f;
    }
    else
    {
        is_opened = false;
        Q_atm = K_atm * Y[PRESSURE];
        atm_flow_sound.state = 1;
        atm_flow_sound.volume = K_sound * cbrt(Q_atm);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoShutoffValve::ode_system(const state_vector_t &Y,
                                    state_vector_t &dYdt,
                                    double t)
{
    (void) t;

    const double ref_pos = static_cast<double>(ref_state.getState());
    const double delta = ref_pos - Y[HANDLE_POS];
    if (abs(delta) > 0.05)
    {
        dYdt[HANDLE_POS] = sign(delta) / switch_time;
    }
    else
    {
        dYdt[HANDLE_POS] = 20.0 * delta / switch_time;
    }

    if (is_opened)
    {
        setY(PRESSURE, p);
        dYdt[PRESSURE] = 0.0;
    }
    else
    {
        dYdt[PRESSURE] = (Q - Q_atm) / V0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoShutoffValve::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    bool state = false;
    cfg.getBool(secName, "IsOpened", state);
    state ? ref_state.set() : ref_state.reset();

    double tmp = 0.0;
    cfg.getDouble(secName, "SwitchTime", tmp);
    if (tmp > 0.01)
        switch_time = tmp;

    tmp = 0.0;
    cfg.getDouble(secName, "V0", tmp);
    if (tmp > 1e-3)
        V0 = tmp;

    cfg.getDouble(secName, "Katm", K_atm);

    cfg.getDouble(secName, "Ksound", K_sound);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoShutoffValve::stepKeysControl(double t, double dt)
{
    ref_state.step(t, dt);
}
