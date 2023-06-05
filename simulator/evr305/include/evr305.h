#ifndef     EVR305_H
#define     EVR305_H

enum
{
    WORK_PRESSURE = 0,  ///< Давление в рабочей камере электровоздухораспределителя

    LINES_NUM = 1,      ///< Количество линий управления ЭПТ
    WORK_LINE = 0,   ///< Линия управления ЭПТ

    VALVES_NUM = 2,     ///< Количество электромагнитных вентилей
    RELEASE_VALVE = 0,  ///< Отпускной электромагнитный вентиль
    BRAKE_VALVE = 1,    ///< Тормозной электромагнитный вентиль

    MAX_COEFFS = 7,
    MAX_LCOEFFS = 3
};

#include    "electro-airdistributor.h"
#include    "pneumo-switching-valve.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EVR305 : public ElectroAirDistributor
{
public:

    EVR305(QObject *parent = Q_NULLPTR);

    ~EVR305();

private:

    /// Объём рабочей камеры
    double V0;

    /// Сопротивление катушек вентилей
    double R;
    /// Индуктивность катушек вентилей
    double L;
    /// Ток включения вентилей
    double I_on;

    std::array<double, MAX_COEFFS> K;
    std::array<double, MAX_LCOEFFS> k;

    SwitchingValve *zpk;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void preStep(state_vector_t &Y, double t);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);

    void step(double t, double dt);
};

#endif // EPK150_H
