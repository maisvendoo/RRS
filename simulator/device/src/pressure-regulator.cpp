#include    "pressure-regulator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PressureRegulator::PressureRegulator(double min_pressure,
                                     double max_pressure,
                                     QObject *parent)
    : Device(parent)
    , hysteresis(new Hysteresis(min_pressure, max_pressure, true))
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PressureRegulator::~PressureRegulator()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PressureRegulator::setPressureRange(double min_pressure, double max_pressure)
{
    hysteresis->setRange(min_pressure, max_pressure);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PressureRegulator::setFLpressure(double fl_pressure)
{
    hysteresis->setValue(fl_pressure);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PressureRegulator::getState() const
{
    return (!hysteresis->getState());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PressureRegulator::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PressureRegulator::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    double min_pressure = 0.75;
    double max_pressure = 0.9;
    cfg.getDouble(secName, "Min_pressure", min_pressure);
    cfg.getDouble(secName, "Max_pressure", max_pressure);

    setPressureRange(min_pressure, max_pressure);
}
