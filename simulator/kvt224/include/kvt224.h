#ifndef KVT224_H
#define KVT224_H

#include    "loco-crane.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    P1_PRESSURE = 0, ///< Давление над переключательным поршнем
    P3_PRESSURE = 1, ///< Давление в межпоршневом пространстве и камере 0.3 литра

    MAX_FLOW_COEFFS = 6,
    MAX_GIAN_COEFFS = 3,
    NUM_STEPS = 5
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class LocoCrane224 : public LocoCrane
{
public:

    LocoCrane224(QObject *parent = Q_NULLPTR);

    ~LocoCrane224();

    void setHandlePosition(double position);

    double getHandlePosition() const;

    double getHandleShift() const;

    void init(double pBP, double pFL);

private:

    /// Объём камеры над переключательным поршнем
    double V1;

    /// Объём межпоршневого пространства с камерой 0.3 литра
    double V3;

    /// Разница давлений, от которой срабатывает переключательный поршень
    double p_switch;

    double min_pos;

    double max_pos;

    double pos_duration;

    /// Текущая фиксированная позиция
    size_t cur_pos;

    std::array<double, NUM_STEPS> step_pressures;

    std::array<double, NUM_STEPS> fixed_pos;

    std::array<double, MAX_FLOW_COEFFS> K;

    std::array<double, MAX_GIAN_COEFFS> k;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);

    /// Изменение номера позиции
    bool isPositionNumberChanged();
};

#endif // KVT224_H
