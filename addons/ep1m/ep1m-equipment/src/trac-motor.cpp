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
  , Rb(0.083)
  , Tf(0.1)
  , beta(0.97)
  , revers_state(1)
  , M(0.0)
  , mode(1)
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

    sound_state.volume = 1.0f;//static_cast<float>(pf(abs(Y[0]) - 100.0) / 1000.0);
    sound_state.pitch = static_cast<float>(abs(omega) / 10.0);

    switch (mode)
    {
    case 1:
        {
            //sound_state.play(true);
            M = Y[0] * cPhi(beta * Y[0] * revers_state) * eff_coef.getValue(Y[0]);

            break;
        }

    case 0:
        {
            //sound_state.play(false);

            M = 0;

            Y[0] = Y[1] = 0.0;

            break;
        }
    case -1:
        {
            //sound_state.play(true);

            M = Y[0] * cPhi(Y[1] * revers_state) * eff_coef.getValue(qAbs(Y[0]));

            break;
        }

    default:

            break;
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

    switch (mode)
    {
    case 1:
        {
            E = cPhi(beta * Y[0] * revers_state) * omega;

            double R = Ra + beta * (Rf + Rd);

            dYdt[0] = (Ua - Y[0] * R - E) / Ta / R;

            dYdt[1] = 0;

            break;
        }

    case -1:
        {
            E = cPhi(Y[1] * revers_state) * omega;

            double R = Ra + Rb;

            dYdt[0] = (Ua - Y[0] * R - E) / Ta / R;

            dYdt[1] = (Uf - Y[1] * (Rf + Rd) ) / Tf / (Rf + Rd);

            break;
        }

    case 0:

    default:
        {
            E = 0;

            dYdt[0] = 0.0;
            dYdt[1] = 0.0;
            break;
        }
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
    cfg.getDouble(secName, "Rb", Rb);
    cfg.getDouble(secName, "Ta", Ta);
    cfg.getDouble(secName, "Tf", Tf);
    cfg.getDouble(secName, "omega_nom", omega_nom);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TractionMotor::cPhi(double If)
{
    return magnetic_char.getValue(If);
}
