#include    "kvt254.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LocoCrane254::LocoCrane254(QObject *parent) : LocoCrane(parent)
  , V1(1e-4)
  , V2(1e-4)
  , Vpz(3e-4)
  , delta_p(0.05)
  , ps(0.1)
{
    std::fill(K.begin(), K.end(), 0.0);
    std::fill(k.begin(), k.end(), 0.0);

    /*DebugLog *log = new DebugLog("kvt254.txt");
    connect(this, &LocoCrane254::DebugPrint, log, &DebugLog::DebugPring);*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LocoCrane254::~LocoCrane254()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double LocoCrane254::getHandlePosition() const
{
    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double LocoCrane254::getAirDistribPressure() const
{
    return getY(0);
}

void LocoCrane254::preStep(state_vector_t &Y, double t)
{
    double k = 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LocoCrane254::ode_system(const state_vector_t &Y,
                              state_vector_t &dYdt,
                              double t)
{
    Q_UNUSED(t)

    // Давление, задаваемое поворотом рукоятки
    double p_handle = K[3] * pf(pos);

    double u1 = hs_p(p_handle - Y[2]);

    double u2 = hs_n(p_handle - Y[2]);

    // Давление, задаваемое уравнительным органом крана
    double pz = p_handle * u1 + Y[2] * u2;

    double dp = pz - pBC;

    double u3 = cut(pf(k[1] * dp), 0.0, 1.0);

    double u4 = cut(nf(k[1] * dp), 0.0, 1.0);

    // Поток воздуха в ТЦ
    Qbc = K[1] * (pFL - pBC) * u3 - K[2] * pBC * u4;

    // Работа повторительной схемы

    double dp12 =  Y[0] - Y[1];

    double u5 = hs_n(dp12 - ps);

    double u6 = hs_n(pos) + is_release;

    double Qpz = K[7] * (Y[1] - Y[2]);

    double Q12 = K[5] * dp12 * u5;

    double Q1 = K[4] * Qvr - K[8] * Q12;

    double Q2 = pf(Q12) - Qpz - K[6] * Y[1] * u6;

    dYdt[0] = Q1 / V1;

    dYdt[1] = Q2 / V2;

    dYdt[2] = Qpz / Vpz;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LocoCrane254::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    for (size_t i = 1; i < K.size(); ++i)
    {
        QString coeff = QString("K%1").arg(i);
        cfg.getDouble(secName, coeff, K[i]);
    }

    for (size_t i = 1; i < k.size(); ++i)
    {
        QString coeff = QString("k%1").arg(i);
        cfg.getDouble(secName, coeff, k[i]);
    }

    cfg.getDouble(secName, "V1", V1);
    cfg.getDouble(secName, "V2", V2);
    cfg.getDouble(secName, "Vpz", Vpz);

    cfg.getDouble(secName, "delta_p", delta_p);

    cfg.getDouble(secName, "ps", ps);
}

GET_LOCO_CRANE(LocoCrane254)
