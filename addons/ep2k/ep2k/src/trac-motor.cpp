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
  , omega(0.0)
  , In(0.0)
  , E(0.0)
  , R_aux(0.75)
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
void TractionMotor::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    E = cPhi(beta * Y[1] * revers_state) * omega;

    M = Y[0] * cPhi(Y[1] * revers_state);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TractionMotor::ode_system(const state_vector_t &Y,
                               state_vector_t &dYdt,
                               double t)
{
    Q_UNUSED(t)


    dYdt[0] = (Ua - Y[0] * (Ra + R_aux) - E) / Ta / (Ra + R_aux);

    dYdt[1] = (Uf - Y[1] * (Rf + Rd)) / Tf / (Rf + Rd);
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
