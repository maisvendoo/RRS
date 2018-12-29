#include    "krm395.h"

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
  , s3(0.0)
{
    std::fill(K.begin(), K.end(), 0.0);
    std::fill(pos.begin(), pos.end(), 0.0);
    pos[POS_II] = 1.0;

    debug_log = new DebugLog("395.log");

    connect(this, &BrakeCrane395::DebugPrint, debug_log, &DebugLog::DebugPring);
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
void BrakeCrane395::setPosition(size_t position)
{
    std::fill(pos.begin(), pos.end(), 0.0);
    pos[position] = 1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane395::ode_system(const state_vector_t &Y,
                               state_vector_t &dYdt,
                               double t)
{
    double T1 = 0.1;

    double T2 = 0.1;

    double u1 = Y[2] - p0;

    double u2 = dead_zone(u1, -0.005, 0.005);

    double u3 = cut(nf(u2), 0.0, 1.0);

    double Qer = K[2] * (Y[2] - Y[ER_PRESSURE]);

    double Qec = K[1] * (pFL - Y[2]) * Y[3] - k_stab * Y[2] - Qer;

    setEqResrvoirFlow(Qer);

    double u4 = A1 * dead_zone(Y[2] - Y[BP_PRESSURE], -0.01, 0.01);

    double u5 = cut(pf(Y[4]), 0.0, 1.0);

    double u6 = cut(nf(Y[4]), 0.0, 1.0);

    double Qbp = K[3] * (pFL - Y[BP_PRESSURE]) * u5 - K[4] * Y[BP_PRESSURE] * u6;

    setBrakePipeFlow(Qbp);

    dYdt[2] = Qec / Vec;

    dYdt[3] = (u3 - Y[3]) / T1;

    dYdt[4] = (u4 - Y[4]) / T2;

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
}


GET_BRAKE_CRANE(BrakeCrane395)
