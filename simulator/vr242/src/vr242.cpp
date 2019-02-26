#include	"vr242.h"

/*
    Y[0] - давление в магистральной камере (МК)
    Y[1] - давление в камере У4
    Y[2] - давление в ЗК
    Y[3] - давление в полости АТ2
    Y[4] - давление в камере У2
*/

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AirDist242::AirDist242() : AirDistributor ()
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
void AirDist242::preStep(const state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    // Перемещение поршня
    double s1 = A1 * (Y[0] - Y[2]);
    // Перемещение клапана доп. разрядки
    double u1 = cut(nf(k[1] * s1), 0.0, 1.0);
    double u2 = cut(pf(k[2] * s1), 0.0, 1.0);

    // Расход воздуха на наполнение ЗР из камеры У4
    double u3 = hs_n(Y[4] - py2);

    Qas = K[9] * (Y[1] - pAS) * u3 - K[4] * (pAS - Y[2]);

    Qbc = K[2] * (Y[3] - pBC);

    // Темп дополнительной разрядки ТМ
    auxRate = nf( - K[5] * Y[0] * u1 / Vmk );

    DebugMsg = QString(" MK: %1 ЗК: %2 Пер. порш.: %3 ТЦ: %4 u1: %5 u2: %6")
            .arg(Y[0], 4, 'f', 2)
            .arg(Y[2], 4, 'f', 2)
            .arg(s1, 7, 'f', 4)
            .arg(pBC, 4, 'f', 2)
            .arg(u1, 7, 'f', 4)
            .arg(u2, 7, 'f', 4);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist242::ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t)
{
    Q_UNUSED(t)

    // Перемещение поршня
    double s1 = A1 * (Y[0] - Y[2]);
    // Перемещение клапана доп. разрядки
    double u1 = cut(nf(k[1] * s1), 0.0, 1.0);
    // Перемещение клапана опорожнения ТЦ
    double u2 = cut(pf(k[2] * s1), 0.0, 1.0);

    // Расход воздуха в камеру У4
    double Qy4 = K[8] * (pTM - Y[1]) - Qas;

    // Расход воздуха в магистральную камеру
    double Qaux = K[5] * Y[0] * u1;

    double Qmk = K[3] * (pTM - Y[0]) - Qaux;

    // Расход воздуха в ЗК
    double Qzk = K[4] * (pAS - Y[2]) - K[6] * (Y[2] - Y[3]) * u1;

    double Qat2 = K[6] * (Y[2] - Y[3]) * u1 - K[10] * Y[3] * u2 - Qbc;

    double Qy2 = K[7] * (Y[3] - Y[4]);

    dYdt[0] = Qmk / Vmk;

    dYdt[1] = Qy4 / Vy4;

    dYdt[2] = Qzk / Vzk;

    dYdt[3] = Qat2 / Vat2;

    dYdt[4] = Qy2 / Vy2;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist242::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    for (size_t i = 0; i < K.size(); ++i)
    {
        QString coeff = QString("K%1").arg(i);
        cfg.getDouble(secName, coeff, K[i]);
    }

    for (size_t i = 0; i < k.size(); ++i)
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
}

GET_AIR_DISTRIBUTOR(AirDist242)
