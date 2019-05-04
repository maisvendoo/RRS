#include    "kvt254.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LocoCrane254::LocoCrane254(QObject *parent) : LocoCrane(parent)
{
    std::fill(K.begin(), K.end(), 0.0);
    std::fill(k.begin(), k.end(), 0.0);
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

    dYdt[0] = Qvr / V1;

    dYdt[1] = 0.0;

    dYdt[2] = 0.0;
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
}

GET_LOCO_CRANE(LocoCrane254)
