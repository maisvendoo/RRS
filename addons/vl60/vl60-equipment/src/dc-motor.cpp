#include    "dc-motor.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCMotor::DCMotor(QObject *parent) : Device(parent)
    , beta(0.95)
    , R_a(0.031)
    , R_gp(0.0238)
    , R_dp(0.032)
    , R_r(0.645)
    , L_af(0.001)
    , omega(0.0)
    , U(0.0)
    , torque(0.0)
    , omega_nom(100.0)
    , direction(1)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCMotor::~DCMotor()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotor::setOmega(double value)
{
    omega = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotor::setU(double value)
{
    U = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DCMotor::getTorque() const
{
    return torque;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DCMotor::getIa() const
{
    return getY(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DCMotor::getIf() const
{
    return getIa() * beta;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DCMotor::getUd() const
{
    return U - getIa() * R_r;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotor::setBeta(double value)
{
    beta = value;
}

void DCMotor::setBetaStep(int step)
{
    if (fieldStep.contains(step))
    {
        setBeta(fieldStep[step]);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotor::setDirection(int revers_state)
{
    if (revers_state == 0)
        return;
    else
        direction = revers_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotor::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    torque = Y[0] * calcCPhi(Y[0] * beta * direction);

    emit soundSetPitch("TED", static_cast<float>(abs(omega) / omega_nom));
    emit soundSetVolume("TED", static_cast<int>(pf(abs(Y[0]) - 300)));
    //DebugMsg = QString("t: %1")
    //        .arg(t, 8, 'f', 3);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotor::ode_system(const state_vector_t &Y,
                         state_vector_t &dYdt,
                         double t)
{
    Q_UNUSED(t)

    double R = R_a + beta * R_gp + R_dp + R_r;
    double E = omega * calcCPhi(Y[0] * beta * direction);

    dYdt[0] = (U - R * Y[0] - E) / L_af;
    //DebugMsg += QString("I: %1, dI: %2, ")
    //        .arg(Y[0], 12, 'f', 5)
    //        .arg(dYdt[0], 12, 'f', 5);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotor::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "R_a", R_a);
    cfg.getDouble(secName, "R_gp", R_gp);
    cfg.getDouble(secName, "R_dp", R_dp);
    cfg.getDouble(secName, "R_r", R_r);
    cfg.getDouble(secName, "L_af", L_af);
    cfg.getDouble(secName, "OmegaNom", omega_nom);

    QString cPhiFileName = "";

    cfg.getString(secName, "cPhi", cPhiFileName);

    cPhi.load((custom_config_dir + QDir::separator() + cPhiFileName).toStdString());

    QDomNode secNode;

    secNode = cfg.getFirstSection("FieldPos");

    while (!secNode.isNull())
    {
        double field_step = 0.95;
        int number = 0;

        cfg.getInt(secNode, "Number", number);
        cfg.getDouble(secNode, "beta", field_step);

        fieldStep.insert(number, field_step);

        secNode = cfg.getNextSection();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DCMotor::calcCPhi(double I)
{
    return cPhi.getValue(I);
}
