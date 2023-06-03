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
  , psv(0.025)
  , pwv(0.48)
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

    setY(0, pBP);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist242::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    // Перемещение главного поршня
    double s1 = dead_zone((pSR - pBP), s1_min, s1_max);

    // Отжатие полого стержня от уплотнения на выпускном клапане (наполнение ТЦ)
    double v11 = cut(k[1] * s1, 0.0, 1.0);

    // Открытие клапана дополнительной разрядки тормозной магистрали
    double v12 = cut(k[2] * s1, 0.0, 1.0);

    // Открытие выпускного клапана
    double v1 = hs_n(s1);

    // Клапаны поршня в камере У2
    double v2 = hs_n(pBC - pu2);

    // Тормозной клапан с учётом режимов торможения/отпуска "К" и "Д"
    double vb = static_cast<double>((!long_train_mode) || (pBC < pbv));

    // Срывной клапан ускорения экстренного торможения
    double vs = static_cast<double>(emergency_mode && ((Y[0] - pBP) > psv));

    // Клапан широкого канала ускорительной камеры при сверхзарядном давлении
    double vw = hs_p(pBP - pwv);

    // Поток из тормозной магистрали в запасный резервуар
    double Q_bp_sr = K[1] * v2 * (pBP - pSR);

    // Поток из тормозной магистрали в ускорительную камеру
    double Q_bp_uk = (K[2] + K[3] * vw) * (pBP - Y[0]);

    // Поток из запасного резервуара в магистраль тормозных цилиндров
    double Q_sr_bc = (K[4] + K[5] * vb) * v11 * (pSR - pBC);

    // Разрядка магистрали тормозных цилиндров в атмосферу
    double Q_bc_atm = ((K[6] + K[7] * vb) * v1 + K[6] * v2) * pBC;

    // Дополнительная разрядка тормозной магистрали
    // в магистраль тормозных цилиндров
    double Q_bp_bc = (K[8] * v12 * v2) * (pBP - pBC);

    // Дополнительная разрядка тормозной магистрали
    // и экстренная разрядка тормозной магистрали в атмосферу
    double Q_bp_atm = ((K[8] * v12 * v2) + (K[9] * vs)) * pBP;

    // Поток в тормозную магистраль
    QBP = - Q_bp_sr - Q_bp_uk - Q_bp_bc - Q_bp_atm;

    // Поток в магистраль тормозных цилиндров
    QBC = Q_bp_bc + Q_sr_bc - Q_bc_atm;

    // Поток в запасный резервуар
    QSR = Q_bp_sr - Q_sr_bc;

    // Поток в ускорительную камеру
    Quk = Q_bp_uk;
/*
//    QString(" pUK   ; pBP   ; pBC   ; pSR   ; BPsr   ; BPuk   ; SRbc   ; BCatm  ; BPatm  ; BPemer ; v11   ; v12   ; v1 ; v2 ; vb ; vs ; vw ");
    DebugMsg = QString("%1;%2;%3;%4;%5;%6;%7;%8;%9;%10;%11;%12;%13;%14;%15;%16;%17")
            .arg(Y[0], 7, 'f', 5)
            .arg(pBP, 7, 'f', 5)
            .arg(pBC, 7, 'f', 5)
            .arg(pSR, 7, 'f', 5)
            .arg(1000*Q_bp_sr, 8, 'f', 5)
            .arg(1000*Q_bp_uk, 8, 'f', 5)
            .arg(1000*Q_sr_bc, 8, 'f', 5)
            .arg(1000*Q_bc_atm, 8, 'f', 5)
            .arg(1000*Q_bp_bc, 8, 'f', 5)
            .arg(1000*Q_bp_atm, 8, 'f', 5)
            .arg(v11, 7, 'f', 5)
            .arg(v12, 7, 'f', 5)
            .arg(v1, 4, 'f', 1)
            .arg(v2, 4, 'f', 1)
            .arg(vb, 4, 'f', 1)
            .arg(vs, 4, 'f', 1)
            .arg(vw, 4, 'f', 1);
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
    cfg.getDouble(secName, "pwv", pwv);

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
