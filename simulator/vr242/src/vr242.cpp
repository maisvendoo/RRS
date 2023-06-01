#include	"vr242.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDist242::AirDist242() : AirDistributor ()
  , long_train_mode(false)
  , emergency_mode(false)
  , Vuk(1.0e-3)
  , Quk(0.0)
  , pu2(0.037)
  , pbv(0.15)
  , psv(0.05)
  , s1_min(-0.015)
  , s1_max(0.015)
{
    K.fill(0.0);
    k.fill(0.0);
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
    double s1 = dead_zone((pBP - pSR), s1_min, s1_max);

    // Перемещение клапана дополнительной разрядки
    double v1 = cut( nf(k[1] * s1), 0.0, 1.0 );

    // Перемещение выпускного клапана
    double v2 = cut( pf(k[2] * s1), 0.0, 1.0 );

    // Клапаны поршня в камере У2
    double v3 = hs_n(pBC - pu2);

    // Тормозной клапан с учётом режимов торможения/отпуска "К" и "Д"
    double v4 = static_cast<double>((!long_train_mode) || (pBC < pbv));

    // Срывной клапан ускорения экстренного торможения
    double v5 = static_cast<double>(emergency_mode && ((pBP - Y[0]) > psv));

    // Поток из тормозной магистрали в запасный резервуар
    double Q_bp_sr = K[1] * v3 * (pBP - pSR);

    // Поток из тормозной магистрали в ускорительную камеру
    double Q_bp_uk = K[2] * (pBP - Y[0]);

    // Разрядка ускорительной камеры в атмосферу
    double Q_uk_atm = K[3] * v5 * Y[0];

    // Поток из запасного резервуара в магистраль тормозных цилиндров
    double Q_sr_bc = (K[4] + K[5] * v4) * v1 * (pSR - pBC);

    // Разрядка магистрали тормозных цилиндров в атмосферу
    double Q_bc_atm = ((K[6] + K[7] * v4) * v2 + K[6] * v3) * pBC;

    // Дополнительная и экстренная разрядка тормозной магистрали в атмосферу
    double Q_bp_atm = (K[8] * v1 * v3 + K[9] * v5) * pBP;

    // Поток в тормозную магистраль
    QBP = - Q_bp_sr - Q_bp_uk - Q_bp_atm;

    // Поток в магистраль тормозных цилиндров
    QBC = Q_sr_bc - Q_bc_atm;

    // Поток в запасный резервуар
    QSR = Q_bp_sr - Q_sr_bc;

    // Поток в ускорительную камеру
    Quk = Q_bp_uk - Q_uk_atm;

    DebugMsg = QString("pUK %1 pBP %2 pBC %3 pSR %4|BPsr %5 SRbc %6 BCatm %7 BPatm %8|v1 %9 v2 %10 v3 %11 v4 %12 v5 %13")
            .arg(Y[0], 7, 'f', 5)
            .arg(pBP, 7, 'f', 5)
            .arg(pBC, 7, 'f', 5)
            .arg(pSR, 7, 'f', 5)
            .arg(1000*Q_bp_sr, 9, 'f', 6)
            .arg(1000*Q_sr_bc, 9, 'f', 6)
            .arg(1000*Q_bc_atm, 9, 'f', 6)
            .arg(1000*Q_bp_atm, 9, 'f', 6)
            .arg(v1, 4, 'f', 1)
            .arg(v2, 4, 'f', 1)
            .arg(v3, 4, 'f', 1)
            .arg(v4, 4, 'f', 1)
            .arg(v5, 4, 'f', 1);

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
    dYdt[0] = Quk / Vuk;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist242::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getBool(secName, "LongTrainMode", long_train_mode);
    cfg.getBool(secName, "EmergencyMode", emergency_mode);

    double tmp = 0.0;
    cfg.getDouble(secName, "Vuk", tmp);
    if (tmp > 1e-3)
        Vuk = tmp;

    cfg.getDouble(secName, "pu2", pu2);
    cfg.getDouble(secName, "pbv", pbv);
    cfg.getDouble(secName, "psv", psv);

    cfg.getDouble(secName, "s1_min", s1_min);
    cfg.getDouble(secName, "s1_max", s1_max);

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

}

GET_AIR_DISTRIBUTOR(AirDist242)
