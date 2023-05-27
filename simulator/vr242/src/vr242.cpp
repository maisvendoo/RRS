#include	"vr242.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDist242::AirDist242() : AirDistributor ()
  , A1(1.0)
  , py2(0.05)
  , s1_min(1e-3)
  , s1_max(1e-3)
  , p_bv(0.15)
  , p_UP(1000.0)
  , long_train(0.0)
  , K2(0.0)
{
    K.fill(0.0);
    k.fill(0.0);
    T.fill(1.0);
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
void AirDist242::init(double pBP, double pFL)
{
    Q_UNUSED(pFL)
    Q_UNUSED(pBP)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist242::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    // Перемещение главного поршня
    double s1 = A1 * (pBP - pSR);
    double s2 = dead_zone(s1, A1 * s1_min, A1 * s1_max);

    // Перемещение клапана дополнительной разрядки
    double v1 = cut( nf(k[1] * s2), 0.0, 1.0 );

    // Перемещение выпускного клапана
    double v2 = cut( pf(k[2] * s2), 0.0, 1.0 );

    // Клапан, разобщающий ТМ и ЗР
    double v3 = hs_n(pBC - py2);

    // Переключение режимов торможения/отпуска ("К" и "Д")
    double p_handle = p_bv + (1 - long_train) * p_UP;
    double v4 = hs_n(pBC - p_handle);

    // Поток из тормозной магистрали в запасный резервуар
    double Q_bp_sr = K[2] * (pBP - pSR) * v3;

    // Поток из запасного резервуара в магистраль тормозных цилиндров
    double Q_sr_bc = (K[4] + K[5] * v4) * (pSR - pBC) * v1;

    // Разрядка магистрали тормозных цилиндров в атмосферу
    double Q_bc_atm = (K[13] + K[15] * v4) * pBC * v2 + K[13] * pBC * v3;

    // Дополнительная разрядки тормозной магистрали в атмосферу
    double Q_bp_atm = K[14] * pBP * v1 * v3;

    // Поток в тормозную магистраль
    QBP = - Q_bp_sr - Q_bp_atm;

    // Поток в магистраль тормозных цилиндров
    QBC = Q_sr_bc - Q_bc_atm;

    // Поток в запасный резервуар
    QSR = Q_bp_sr - Q_sr_bc;
/*
    DebugMsg = QString("pBP %1 pBC %2 pSR %3|BPsr %4 SRbc %5 BCatm %6 BPatm %7|v1 %8 v2 %9 v3 %10 v4 %11")
            .arg(pBP, 7, 'f', 5)
            .arg(pBC, 7, 'f', 5)
            .arg(pSR, 7, 'f', 5)
            .arg(100000*Q_bp_sr, 6, 'f', 3)
            .arg(100000*Q_sr_bc, 6, 'f', 3)
            .arg(100000*Q_bc_atm, 6, 'f', 3)
            .arg(100000*Q_bp_atm, 6, 'f', 3)
            .arg(v1, 4, 'f', 1)
            .arg(v2, 4, 'f', 1)
            .arg(v3, 4, 'f', 1)
            .arg(v4, 4, 'f', 1);
*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist242::ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
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

    for (size_t i = 0; i < T.size(); ++i)
    {
        QString coeff = QString("T%1").arg(i);
        cfg.getDouble(secName, coeff, T[i]);
    }

    cfg.getDouble(secName, "A1", A1);

    cfg.getDouble(secName, "Py2", py2);

    cfg.getDouble(secName, "s1_min", s1_min);
    cfg.getDouble(secName, "s1_max", s1_max);

    cfg.getDouble(secName, "p_bv", p_bv);
    cfg.getDouble(secName, "LongTrainReg", long_train);
}

GET_AIR_DISTRIBUTOR(AirDist242)
