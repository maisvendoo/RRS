#include    "krm395.h"

#include    <iostream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeCrane395::BrakeCrane395(QObject *parent) : BrakeCrane (parent)
  , k_leek(0.0)
  , k_stab(0.0)
  , Vec(1.0)
  , A1(1.0)
  , k1(1.0)
  , k2(1.0)
  , k3(1.0)
  , T1(0.1)
  , T2(0.1)
  , K4_power(10.0)
  , Qbp(0.0)
  , old_input(0)
  , old_output(0)
  , pulse_II(true)
{
    std::fill(K.begin(), K.end(), 0.0);
    std::fill(pos.begin(), pos.end(), 0.0);
    pos[POS_II] = 1.0;

    positions_names << "I" << "II" << "III" << "IV" << "Va" << "V" << "VI";

    float max_angle = 115.0f;
    positions.push_back(0.0f);
    positions.push_back(35.0f / max_angle);
    positions.push_back(52.0f / max_angle);
    positions.push_back(59.0f / max_angle);
    positions.push_back(63.0f / max_angle);
    positions.push_back(80.0f / max_angle);
    positions.push_back(1.0f);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeCrane395::~BrakeCrane395()
{
    delete debug_log;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane395::setPosition(int &position)
{
    position = cut(position, static_cast<int>(POS_I), static_cast<int>(POS_VI));

    std::fill(pos.begin(), pos.end(), 0.0);
    pos[static_cast<size_t>(position)] = 1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString BrakeCrane395::getPositionName(int position)
{
    return positions_names[position];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float BrakeCrane395::getHandlePosition(int position)
{
    size_t pos = static_cast<size_t>(position);
    return positions[pos];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane395::init(double pTM)
{
    y[0] = pTM;
    y[1] = pTM;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane395::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    if ( (static_cast<int>(pos[POS_II]) == 1) && pulse_II )
    {
        Y[0] = cut(pf(2 * p0  - Y[1]), 0.0, pFL);
        pulse_II = false;
    }
    else
    {
        pulse_II = true;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane395::ode_system(const state_vector_t &Y,
                               state_vector_t &dYdt,
                               double t)
{
    double Q_charge = K[1] * (pFL - Y[1]) * pos[POS_I];

    double u1 = hs_n(Y[1] - p0);

    double Q_train = K[1] * (pFL - Y[1]) * u1 * pos[POS_II];

    double Q_stab = - k_stab * cut(k1 * Y[1], 0.0, 1.0) * pos[POS_II];

    double Q_brake = - K[5] * Y[1] * pos[POS_Va]
                     - K[6] * Y[1] * pos[POS_V]
                     - K[9] * Y[1] * pos[POS_VI];

    double Qer = Q_charge + Q_train + Q_stab + Q_brake;

    double s1 = A1 * (Y[0] - Y[1]);

    double K4 = 0;

    K4 = K[4] * (1.0 + pow(pf(Y[BP_PRESSURE] / p0 - 1.0), K4_power));

    double u2 = cut(k2 * nf(s1), 0.0, 1.0) * (1.0 - pos[POS_VI]);
    double u3 = cut(k3 * pf(s1), 0.0, 1.0);

    double Qbp =   K[2] * (pFL - Y[0]) * u2
                 - K4 * Y[0] * u3
                 - K[7] * Y[0] * pos[POS_VI]
                 + K[8] * (pFL - Y[0]) * pos[POS_I];

    setBrakePipeFlow(Qbp);
    setEqResrvoirFlow(Qer);

    BrakeCrane::ode_system(Y, dYdt, t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane395::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    for (size_t i = 1; i <= MAX_FLOW_COEFFS; ++i)
    {
        QString param = QString("K%1").arg(i);

        cfg.getDouble(secName, param, K[i]);
    }

    cfg.getDouble(secName, "EqReservoirVolume", Ver);
    cfg.getDouble(secName, "LocoBrakePipeVolume", Vbp);
    cfg.getDouble(secName, "EqCameraVolume", Vec);
    cfg.getDouble(secName, "k_leek", k_leek);
    cfg.getDouble(secName, "k_stab", k_stab);
    cfg.getDouble(secName, "A1", A1);
    cfg.getDouble(secName, "k1", k1);
    cfg.getDouble(secName, "k2", k2);
    cfg.getDouble(secName, "k3", k3);
    cfg.getDouble(secName, "T1", T1);
    cfg.getDouble(secName, "T2", T2);
    cfg.getDouble(secName, "K4_power", K4_power);
}


GET_BRAKE_CRANE(BrakeCrane395)
