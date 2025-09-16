#include    "pneumo-electro-valve.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoElectroValve::PneumoElectroValve(QObject *parent) : Device(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoElectroValve::~PneumoElectroValve()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoElectroValve::setVoltage(double value)
{
    U = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoElectroValve::getCurrent() const
{
    return getY(CURRENT);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PneumoElectroValve::isOpened() const
{
    return hysteresis.getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoElectroValve::setPipePressure(double value)
{
    p = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoElectroValve::getPressureToDevice() const
{
    return getY(PRESSURE);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoElectroValve::setDeviceFlow(double value)
{
    Q = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoElectroValve::getFlowToPipe() const
{
    if (hysteresis.getState())
        return Q;
    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t PneumoElectroValve::getSoundState(size_t idx) const
{
    if (idx == DRAIN_FLOW_SOUND)
        return atm_flow_sound;
    return hysteresis.getSoundState(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float PneumoElectroValve::getSoundSignal(size_t idx) const
{
    if (idx == DRAIN_FLOW_SOUND)
        return atm_flow_sound.createSoundSignal();
    return hysteresis.getSoundSignal(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoElectroValve::preStep(state_vector_t &Y, double t)
{
    (void) t;

    hysteresis.setValue(Y[CURRENT]);

    if (hysteresis.getState())
    {
        Q_atm = 0.0;
        atm_flow_sound.state = 0;
        atm_flow_sound.volume = 0.0f;
    }
    else
    {
        Q_atm = K_atm * Y[PRESSURE];
        atm_flow_sound.state = 1;
        atm_flow_sound.volume = K_sound * cbrt(Q_atm);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoElectroValve::ode_system(const state_vector_t &Y,
                                    state_vector_t &dYdt,
                                    double t)
{
    (void) t;

    if (hysteresis.getState())
    {
        setY(PRESSURE, p);
        dYdt[PRESSURE] = 0.0;
    }
    else
    {
        dYdt[PRESSURE] = (Q - Q_atm) / V0;
    }

    dYdt[CURRENT] = (abs(U) / R - Y[CURRENT]) / T;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoElectroValve::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "U_nom", U_nom);

    double tmp = 0.0;
    cfg.getDouble(secName, "R", tmp);
    if (tmp > Physics::ZERO)
        R = tmp;

    cfg.getDouble(secName, "T", T);

    cfg.getDouble(secName, "level_off", level_off);
    double I_off = U_nom * level_off / R;

    cfg.getDouble(secName, "level_on", level_on);
    double I_on = U_nom * level_on / R;

    hysteresis.setRange(I_off, I_on);

    tmp = 0.0;
    cfg.getDouble(secName, "V0", tmp);
    if (tmp > 1e-3)
        V0 = tmp;

    cfg.getDouble(secName, "Katm", K_atm);

    cfg.getDouble(secName, "Ksound", K_sound);
}
