#include	"vr242.h"

/*
    Y[0] - давление в магистральной камере (МК)
    Y[1] - давление в камере У2
*/

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDist242::AirDist242() : AirDistributor ()
{
    K.fill(0.0);
    k.fill(0.0);

    s1_min = 1e-3;
    s1_max = 1e-3;

    //DebugLog *log = new DebugLog("vr242.txt");
    //connect(this, &AirDist242::DebugPrint, log, &DebugLog::DebugPring);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDist242::~AirDist242()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist242::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    double s1 = A1 * (Y[0] - pAS);
    double s2 = dead_zone(s1, s1_min, s1_max);

    double v1 = cut(nf(k[1] * s2), 0.0, 1.0);
    double v2 = cut(pf(k[1] * s2), 0.0, 1.0);

    double v3 = hs_n(Y[1] - py2);

    Qas = K[2] * (Y[0] - pAS) * v3 - pf( 2.0 * Qbc);

    Qbc = K[4] * (pAS - pBC) * v1 - K[4] * pBC * v2;// - K[7] * pBC * v3;

    DebugMsg = QString(" МК: %1 ТЦ: %3 s1: %2 v1: %4 v2: %5")
            .arg(Y[0], 4, 'f', 2)
            .arg(s1, 7, 'f', 4)
            .arg(pBC, 4, 'f', 2)
            .arg(v1, 7, 'f', 4)
            .arg(v2, 7, 'f', 4);


    //auxRate = Qas / 0.078;

    Y[2] = pAS;
    Y[3] = pBC;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist242::ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t)
{
    Q_UNUSED(t)

    double s1 = A1 * (Y[0] - pAS);
    double s2 = dead_zone(s1, s1_min, s1_max);
    double v1 = cut(nf(k[1] * s2), 0.0, 1.0);
    double v2 = cut(pf(k[1] * s2), 0.0, 1.0);

    double v3 = hs_n(Y[1] - py2);

    double Qmk = K[1] * (pTM - Y[0])
            - K[3] * Y[0] * v1 * v3
            - K[2] * (Y[0] - pAS) * v3;

    double Qy2 = K[6] * (pAS - Y[1]) * v1 - K[6] * Y[1] * v2;

    dYdt[0] = Qmk / Vmk;

    dYdt[1] = Qy2 / Vy2;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist242::load_config(CfgReader &cfg)
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

    cfg.getDouble(secName, "Vmk", Vmk);
    cfg.getDouble(secName, "Vy4", Vy4);
    cfg.getDouble(secName, "Vzk", Vzk);
    cfg.getDouble(secName, "Vat2", Vat2);
    cfg.getDouble(secName, "Vy2", Vy2);
    cfg.getDouble(secName, "A1", A1);

    cfg.getDouble(secName, "Py2", py2);

    cfg.getDouble(secName, "s1_min", s1_min);
    cfg.getDouble(secName, "s1_max", s1_max);
}

GET_AIR_DISTRIBUTOR(AirDist242)
