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
    T.fill(1.0);

    s1_min = 1e-3;
    s1_max = 1e-3;

    p_bv = 0.15;
    p_UP = 1000.0;

    long_train = 0.0;

    K2 = 0;

    /*DebugLog *log = new DebugLog("vr242.txt");
    connect(this, &AirDist242::DebugPrint, log, &DebugLog::DebugPring);*/
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
void AirDist242::init(double pTM, double pFL)
{
    Q_UNUSED(pFL)

    y[0] = pTM;
    y[1] = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AirDist242::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    Y[2] = pAS;
    Y[3] = pBC;

    // Перемещение главного поршня
    double s1 = A1 * (pTM - pAS);
    double s2 = dead_zone(s1, A1 * s1_min, A1 * s1_max);

    // Перемещение клапана дополнительной зазрядки
    double v1 = cut( nf(k[1] * s2), 0.0, 1.0 );

    // Перемещение выпускного клапана
    double v2 = cut( pf(k[2] * s2), 0.0, 1.0 );

    // Клапан, разобщающий ТМ и ЗР
    double v3 = hs_n(pBC - py2);

    // Переключение режимов торможения/отпуска ("К" и "Д")
    double p_handle = p_bv + (1 - long_train) * p_UP;
    double v4 = hs_n(pBC - p_handle);

    // Расход воздуха в ТЦ
    Qbc = (K[4] + K[5] * v4) * (pAS - pBC) * v1 - (K[13] + K[15] * v4) * pBC * v2 - K[13] * pBC * v3;

    // Расход воздуха в ЗР
    Qas = K[2] * (pTM - pAS) * v3 - pf( K[12] * Qbc );

    // Темп дополнительной разрядки ТМ
    auxRate = K[14] * pTM * v1 * v3;
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

    cfg.getDouble(secName, "Vmk", Vmk);
    cfg.getDouble(secName, "Vy4", Vy4);
    cfg.getDouble(secName, "Vzk", Vzk);
    cfg.getDouble(secName, "Vat2", Vat2);
    cfg.getDouble(secName, "Vy2", Vy2);
    cfg.getDouble(secName, "A1", A1);

    cfg.getDouble(secName, "Py2", py2);

    cfg.getDouble(secName, "s1_min", s1_min);
    cfg.getDouble(secName, "s1_max", s1_max);

    cfg.getDouble(secName, "p_bv", p_bv);
    cfg.getDouble(secName, "LongTrainReg", long_train);
}

GET_AIR_DISTRIBUTOR(AirDist242)
