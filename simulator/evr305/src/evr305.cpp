#include    "evr305.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EVR305::EVR305(QObject *parent)
    : ElectroAirDistributor(parent)
    , A1(0.0)
    , Vpk(0.0)
    , Q1(0.0)
    , Ip_max(0.2)
    , It_max(0.2)
{
    zpk = new SwitchingValve();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EVR305::~EVR305()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EVR305::ode_system(const state_vector_t &Y,
                        state_vector_t &dYdt,
                        double t)
{
    Q_UNUSED(t)

    // Состояние вентиля перекрыши
    double up = abs(control_line[0]);

    // Состояние тормозного вентиля
    double ut = pf(control_line[0]);

    // Поток воздуха из ЗР в РК
    double Q_sr_work = K[1] * (pSR - Y[WORK_PRESSURE]) * up * ut;

    // Поток воздуха из РК а атмосферу
    double Q_work_atm = K[2] * Y[WORK_PRESSURE] * (1.0 - up);

    // Перемещение диафрагмы реле давления
    double s = A1 * (Y[WORK_PRESSURE] - pBC);

    // Состояние клапана наполнения ТЦ
    double u1 = cut(pf(k[1] * s), 0.0, 1.0);

    // Состояние клапана опорожнения ТЦ
    double u2 = cut(nf(k[2] * s), 0.0, 1.0);

    // Поток воздуха на заполнение ТЦ
    double p1 = zpk->getPressure1();
    double Q_sr_bc = K[4] * (pSR - p1) * u1;

    // Поток воздуха на опорожнение ТЦ
    double Q_bc_atm = K[6] * p1 * u2;

    // Работа с ТЦ через переключательный клапан
    zpk->setInputFlow1(Q_sr_bc - Q_bc_atm);
    zpk->setInputFlow2(Q_airdistBC);
    p_airdistBC = zpk->getPressure2();

    zpk->setOutputPressure(pBC);
    QBC = zpk->getOutputFlow();

    QSR = Q_airdistSR - K[3] * Q_sr_work - K[5] * Q_sr_bc;

    p_airdistSR = pSR;

    // Поток в рабочую камеру
    dYdt[0] = (Q_sr_work - Q_work_atm) / Vpk;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EVR305::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    valve_state[0] = Ip_max * abs(control_line[0]);
    valve_state[1] = It_max * pf(control_line[0]);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EVR305::load_config(CfgReader &cfg)
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

    cfg.getDouble(secName, "A1", A1);

    cfg.getDouble(secName, "Vpk", Vpk);

    cfg.getDouble(secName, "Ip_max", Ip_max);

    cfg.getDouble(secName, "It_max", It_max);

    zpk->read_config("zpk");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EVR305::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EVR305::step(double t, double dt)
{
    zpk->step(t, dt);

    ElectroAirDistributor::step(t, dt);
}

GET_ELECTRO_AIRDISTRIBUTOR(EVR305)
