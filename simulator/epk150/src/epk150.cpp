//------------------------------------------------------------------------------
//
//      Электро-пневматический клапан автостопа (ЭПК) усл. №150
//      (c) maisvendoo, 06/05/2019
//
//------------------------------------------------------------------------------

#include    <epk150.h>
/*
#include    <Journal.h>
*/
#include    "physics.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AutoTrainStopEPK150::AutoTrainStopEPK150(QObject *parent)
    : AutoTrainStop(parent)
{
    std::fill(K.begin(), K.end(), 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AutoTrainStopEPK150::~AutoTrainStopEPK150()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStopEPK150::init(double pBP, double pFL)
{
    setY(COIL_FORCE, is_powered ? pk : 0.0);
    setY(P_ABOVE_FAILURE_VALVE, pBP);
    setY(P_TIME_DELAY, pFL);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double AutoTrainStopEPK150::getPressureAboveFailureValve() const
{
    return getY(P_ABOVE_FAILURE_VALVE);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStopEPK150::ode_system(const state_vector_t &Y,
                                     state_vector_t &dYdt,
                                     double t)
{
    Q_UNUSED(t)

    // Баланс сил на мембране камеры выдержки времени
    double dp1 = Y[P_TIME_DELAY] - ps2;

    double u1 = std::clamp(pf(A[1] * dp1), 0.0, 1.0);

    double u2 = std::clamp(nf(A[2] * dp1), 0.0, 1.0);

    // Баланс сил на срывном клапане
    double sum_p1 = Y[P_ABOVE_FAILURE_VALVE] + ps1 - pBP;

    double u3 = std::clamp(nf(A[3] * sum_p1), 0.0, 1.0);
/*
    bool old_emergency_brake = is_emergency_brake;
*/
    // Имитируем опускание диафрагмы в камере выдержки времени
    is_emergency_brake = Y[P_TIME_DELAY] <= 0.15;
/*
    if (is_emergency_brake != old_emergency_brake)
    {
        if (is_emergency_brake)
            Journal::instance()->info("EPK emergency brake!!!");
        else
            Journal::instance()->info("EPK is released...");
    }
*/
    // Баланс сил на диафрагме плунжера
    double sum_p2 = (isKeyOn() ? Y[COIL_FORCE] : pk) - pd;

    is_whistle = (sum_p2 < -Physics::ZERO);

    // Поток из питательной магистрали в камеру выдержки времени
    double Q_fl_2 = K[4] * (pFL - Y[P_TIME_DELAY]) * hs_p(sum_p2);

    // Разрядка камеры выдержки времени в атмосферу
    double Q_2_atm = K[5] * Y[P_TIME_DELAY] * static_cast<double>(is_whistle);

    // Поток из тормозной магистрали в камеру над срывным клапаном
    double Q_bp_1 = K[1] * (pBP - Y[P_ABOVE_FAILURE_VALVE]);

    // Разрядка камеры над срывным клапаном в атмосферу
    double Q_1_atm = K[2] * Y[P_ABOVE_FAILURE_VALVE] * u2;

    // Экстренная разрядка тормозной магистрали в атмосферу
    double Q_bp_emergency = K[3] * pBP * u3;

    // Поток в питательную магистраль
    QFL = - Q_fl_2;

    // Поток в тормозную магистраль
    QBP = - Q_bp_1 - Q_bp_emergency;

    // Изменение силы в электромагните (катушке)
    dYdt[COIL_FORCE] = ( static_cast<double>(is_powered) * pk * u1 - Y[COIL_FORCE] ) / T1;

    // Поток в камеру над срывным клапаном
    dYdt[P_ABOVE_FAILURE_VALVE] = (Qabove_failure_valve + Q_bp_1 - Q_1_atm) / V1;

    // Поток в камеру выдержки времени
    dYdt[P_TIME_DELAY] = (Q_fl_2 - Q_2_atm) / V2;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStopEPK150::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    for (size_t i = 1; i < K.size(); ++i)
    {
        QString coeff = QString("K%1").arg(i);
        cfg.getDouble(secName, coeff, K[i]);
    }

    for (size_t i = 1; i < A.size(); ++i)
    {
        QString coeff = QString("A%1").arg(i);
        cfg.getDouble(secName, coeff, A[i]);
    }

    cfg.getDouble(secName, "T1", T1);
    cfg.getDouble(secName, "pd", pd);
    cfg.getDouble(secName, "pk", pk);
    cfg.getDouble(secName, "p_key", p_key);
    cfg.getDouble(secName, "ps1", ps1);
    cfg.getDouble(secName, "ps2", ps2);
    cfg.getDouble(secName, "V1", V1);
    cfg.getDouble(secName, "V2", V2);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_AUTO_TRAIN_STOP(AutoTrainStopEPK150)
