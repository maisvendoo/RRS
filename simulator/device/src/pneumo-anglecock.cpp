#include    "pneumo-anglecock.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoAngleCock::PneumoAngleCock(int key_code, QObject *parent) : Device(parent)
  , keyCode(key_code)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoAngleCock::~PneumoAngleCock()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::setKeyCode(int key_code)
{
    keyCode = key_code;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::close()
{
    ref_state.reset();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::open()
{
    ref_state.set();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PneumoAngleCock::isOpened() const
{
    return is_opened;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoAngleCock::getHandlePosition() const
{
    return getY(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::setPipeVolume(double value)
{
    pipe_volume = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::setPipePressure(double value)
{
    p = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::setHoseFlow(double value)
{
    Q = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::setShiftSide(double value)
{
    shift_side = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::setShiftCoord(double value)
{
    shift_coord = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoAngleCock::getFlowCoeff() const
{
    if (is_opened)
        return k_max_by_pipe_volume * getY(0);
    return k_atm;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoAngleCock::getFlowToPipe() const
{
    if (is_opened)
        return Q;
    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoAngleCock::getPressureToHose() const
{
    if (is_opened)
        return p;
    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoAngleCock::getShiftSide() const
{
    return shift_side;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoAngleCock::getShiftCoord() const
{
    return shift_coord;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t PneumoAngleCock::getSoundState(size_t idx) const
{
    if (idx == PIPE_DRAIN_FLOW_SOUND)
        return atm_flow_sound;
    return ref_state.getSoundState(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float PneumoAngleCock::getSoundSignal(size_t idx) const
{
    if (idx == PIPE_DRAIN_FLOW_SOUND)
        return atm_flow_sound.createSoundSignal();
    return ref_state.getSoundSignal(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    Q_UNUSED(t)

    double ref_pos = static_cast<double>(ref_state.getState());
    double delta = ref_pos - Y[0];
    if (abs(delta) > 0.05)
    {
        dYdt[0] = sign(delta) / switch_time;
    }
    else
    {
        dYdt[0] = 20.0 * delta / switch_time;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)
    if (Y[0] < 0.05)
    {
        is_opened = false;
        atm_flow_sound.state = 1;
        atm_flow_sound.volume = K_sound * cbrt(Q);
    }
    else
    {
        is_opened = true;
        atm_flow_sound.state = 0;
        atm_flow_sound.volume = 0.0f;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)

    // Ограничение коэффициента перетока для устойчивого расчёта с данным шагом
    k_max_by_pipe_volume = min(k_pipe, (0.5 * pipe_volume / dt) );

    // Проверяем управляющий сигнал
    if (getKeyState(keyCode))
    {
        if (isShift() && (!isControl()))
        {
            // Открываем концевой кран
            open();
        }
        else
        {
            if (isControl())
                // Закрываем концевой кран
                close();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    bool state = false;
    cfg.getBool(secName, "IsOpened", state);
    state ? ref_state.set() : ref_state.reset();

    double tmp = 0.0;
    cfg.getDouble(secName, "SwitchTime", tmp);
    if (tmp > 0.1)
        switch_time = tmp;

    cfg.getDouble(secName, "kPipe", k_pipe);
    cfg.getDouble(secName, "kAtm", k_atm);
    cfg.getDouble(secName, "K_sound", K_sound);

    k_max_by_pipe_volume = k_pipe;

    cfg.getDouble(secName, "ShiftSide", shift_side);
    cfg.getDouble(secName, "ShiftCoord", shift_coord);
}
