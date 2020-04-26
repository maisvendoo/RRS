#include    "dc-motor-compressor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCMotorCompressor::DCMotorCompressor(QObject *parent) : Device(parent)

  , p(0.0)
  , Q(0.0)
  , U_power(0.0)
  , omega0(104.7)

  , Mxx(50.0)
  , J(0.5)

  , R(0.089)
  , U(0.0)
  , cPhi(0.85)
  , I(0.0)
  , Ma(0.0)
  , Vnk(0.05)
  , soundName("")
  , rd(0.0)

{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCMotorCompressor::~DCMotorCompressor()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::setSoundName(const QString &value)
{
    soundName = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    rd = 0.0;

    for (size_t i = 0; i < Rd.size(); ++i)
    {
        rd += static_cast<double>(kontaktor_step[i]) * Rd[i];
    }

    Q = K[4] * pf(Y[1] - p);

    emit soundSetPitch(soundName, static_cast<float>(Y[0] / omega0));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::ode_system(const state_vector_t &Y,
                                 state_vector_t &dYdt,
                                 double t)
{
    Q_UNUSED(t)

    double Mr = Physics::fricForce(Mxx, Y[0]);

    I = (U - cPhi * Y[0]) / (R + rd);

    Ma = cPhi * I;

    double Qnk =  K[1] * Y[0] - K[2] * Y[1] - K[3] * pf(Y[1] - p);

    dYdt[0] = (Ma - Mr) / J;


    dYdt[1] = Qnk / Vnk;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    std::fill(K.begin(), K.end(), 0);

    for (size_t i = 1; i < K.size(); ++i)
    {
        QString coeff = QString("K%1").arg(i);
        cfg.getDouble(secName, coeff, K[i]);
    }

    QString rd_list = "";
    cfg.getString(secName, "Rd", rd_list);

    if (!rd_list.isEmpty())
    {
        QStringList resistors = rd_list.split(' ');

        for (QString res : resistors)
        {
            Rd.push_back(res.toDouble());
            kontaktor_step.push_back(true);
        }
    }
}
