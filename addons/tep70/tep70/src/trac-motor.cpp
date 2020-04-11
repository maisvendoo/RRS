#include    "trac-motor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TractionMotor::TractionMotor(QObject *parent) : Device(parent)
  , Ua(0.0)
  , Uf(0.0)
  , Ra(0.0113)
  , Ta(0.5)
  , Rf(0.0068)
  , Rd(0.0065)
  , Tf(0.1)
  , beta(1.0)
  , revers_state(1)
  , M(0.0)
  , is_motor(true)
  , omega(0.0)
  , In(0.0)
  , E(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TractionMotor::~TractionMotor()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TractionMotor::load_magnetic_char(QString file_name)
{
    magnetic_char.load(file_name.toStdString());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TractionMotor::load_eff_coeff(QString file_name)
{
    eff_coef.load(file_name.toStdString());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TractionMotor::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    if (is_motor)
    {
        E = cPhi(beta * Y[0] * revers_state) * omega;

        M = Y[0] * cPhi(beta * Y[0] * revers_state);
    }
    else
    {

    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TractionMotor::ode_system(const state_vector_t &Y,
                               state_vector_t &dYdt,
                               double t)
{
    Q_UNUSED(t)

    if (is_motor)
    {
        double R = Ra + beta * (Rf + Rd);        

        dYdt[0] = (Ua - Y[0] * R - E) / Ta / Ra;

        dYdt[1] = 0;
    }
    else
    {

    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TractionMotor::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "Ra", Ra);
    cfg.getDouble(secName, "Rf", Rf);
    cfg.getDouble(secName, "Rd", Rd);
    cfg.getDouble(secName, "Ta", Ta);
    cfg.getDouble(secName, "Tf", Tf);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TractionMotor::cPhi(double If)
{
    return magnetic_char.getValue(If);
}
