//------------------------------------------------------------------------------
//
//      Электро-пневматический клапан автостопа (ЭПК) усл. №150
//      (c) maisvendoo, 06/05/2019
//
//------------------------------------------------------------------------------

#ifndef     EPK150_H
#define     EPK150_H

#include    "automatic-train-stop.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AutoTrainStopEPK150 : public AutoTrainStop
{
public:

    AutoTrainStopEPK150(QObject *parent = nullptr);

    ~AutoTrainStopEPK150();

    void init(double pBP, double pFL) override;

    double getPressureAboveFailureValve() const override;

private:

    enum {
        MAX_FLOW_COEFFS = 6,
        MAX_GIAN_COEFFS = 4,

        COIL_FORCE = 0,             ///< Y[0] - Усилие от катушки ЭПК
        P_ABOVE_FAILURE_VALVE = 1,  ///< Y[1] - Давление над срывным клапаном
        P_TIME_DELAY = 2            ///< Y[2] - Давление в камере выдержки времени
    };

    /// Постоянная времени срабатывания электромеханической части (катушки)
    double T1 = 0.1;

    /// Усилие от диафрагмы плунжера
    double pd = 0.2;

    /// Усилие, развиваемое электромагнитом катушки
    double pk = 0.4;

    /// Усилие от ключа
    double p_key = 10.0;

    /// Усилие пружины срывного клапана
    double ps1 = 0.1;

    /// Усилие от пружины мембраны камеры выдежки времени
    double ps2 = 0.15;

    /// Объем камеры над срывным клапаном
    double V1 = 1.0e-4;

    /// Объем камеры выдержки времени
    double V2 = 1.0e-3;

    std::array<double, MAX_FLOW_COEFFS> K;

    std::array<double, MAX_GIAN_COEFFS> A;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    void load_config(CfgReader &cfg) override;
};

#endif // EPK150_H
