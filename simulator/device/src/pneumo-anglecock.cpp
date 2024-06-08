#include    "pneumo-anglecock.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoAngleCock::PneumoAngleCock(int key_code, QObject *parent) : Device(parent)
  , keyCode(key_code)
  , ref_state(false)
  , switch_time(0.2)
  , p(0.0)
  , Q(0.0)
  , pipe_volume(1.0e8)
  , k_max_by_pipe_volume(0.8)
  , k_pipe(0.8)
  , k_atm(0.1)
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
    ref_state = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::open()
{
    ref_state = true;
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
void PneumoAngleCock::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    Q_UNUSED(t)

    double ref_pos = static_cast<double>(ref_state);
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
void PneumoAngleCock::stepDiscrete(double t, double dt)
{
    Q_UNUSED(t)

    // Ограничение коэффициента перетока для устойчивого расчёта с данным шагом
    k_max_by_pipe_volume = min(k_pipe, (0.5 * pipe_volume / dt) );
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)
    if (Y[0] < 0.05)
        is_opened = false;
    else
        is_opened = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    // Проверяем управляющий сигнал
    if (getKeyState(keyCode))
    {
        if (isShift())
            // Открываем концевой кран
            open();
        else
            // Закрываем концевой кран
            close();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoAngleCock::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getBool(secName, "IsOpened", ref_state);

    double tmp = 0.0;
    cfg.getDouble(secName, "SwitchTime", tmp);
    if (tmp > 0.1)
        switch_time = tmp;

    cfg.getDouble(secName, "kPipe", k_pipe);
    cfg.getDouble(secName, "kAtm", k_atm);

    k_max_by_pipe_volume = k_pipe;

    cfg.getDouble(secName, "ShiftSide", shift_side);
    cfg.getDouble(secName, "ShiftCoord", shift_coord);
}
