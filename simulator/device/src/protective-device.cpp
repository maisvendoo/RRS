#include    "protective-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProtectiveDevice::ProtectiveDevice(QObject *parent) : Device(parent)
  , U_in(0.0)
  , U_out(0.0)
  , is_on(false)
  , is_return(false)
  , holding_coil(false)
  , Vn(3.33)
  , Fp(1.0)
  , Fk(1.1)
  , Vdk(1e-3)
  , K1(1.0)
  , K2(1e-2)
  , p0(0.5)
  , p1(0.3)
  , lamp_state(1.0f)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProtectiveDevice::~ProtectiveDevice()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProtectiveDevice::setU_in(double value)
{
    U_in = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProtectiveDevice::setState(bool is_on)
{
    this->is_on = is_on;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProtectiveDevice::setReturn(bool is_return)
{
    this->is_return = is_return;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProtectiveDevice::setHoldingCoilState(bool holding_coil)
{
    this->holding_coil = holding_coil;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ProtectiveDevice::getKnifePos() const
{
    return getY(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float ProtectiveDevice::getLampState() const
{
    return lamp_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProtectiveDevice::getState() const
{
    return state.getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t ProtectiveDevice::getSoundOn() const
{
    return state.getSoundState(Trigger::ON_SOUND);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t ProtectiveDevice::getSoundOff() const
{
    return state.getSoundState(Trigger::OFF_SOUND);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProtectiveDevice::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    if (Y[0] >= 1.0)
    {
        state.set();
        lamp_state = 0.0f;
    }
    else
    {
        state.reset();
        lamp_state = 1.0f;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ProtectiveDevice::getU_out() const
{
    return U_out;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProtectiveDevice::ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t)
{
    Q_UNUSED(t)

    double s_on_off = static_cast<double>(is_on);

    double s_on = static_cast<double>(is_return) * s_on_off;
    double s_off = 1.0 - s_on_off;

    double s_hc = static_cast<double>(holding_coil);

    double dx = Y[0] - 1.0;
    double s2 = s_on * hs_n(dx);

    double s5 = hs_p(Fp * pf(Y[0]) - Fk * s_on_off * s_hc);

    double s3 = (s_off + s5) * hs_p(Y[0]);

    double s1 = s2 - s3;

    U_out = U_in * hs_p(dx);    

    dYdt[0] = s1 * Vn;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProtectiveDevice::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)

    QString secName = "Device";

    int order = 1;
    if (cfg.getInt(secName, "Order", order))
    {
        y.resize(static_cast<size_t>(order));
        std::fill(y.begin(), y.end(), 0);
    }
}
