#ifndef     KVT254_H
#define     KVT254_H

#include    "loco-crane.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    P1_PRESSURE = 0, ///< Давление над переключательным поршнем
    P3_PRESSURE = 1, ///< Давление в межпоршневом пространстве и камере 0.3 литра

    MAX_FLOW_COEFFS = 10,
    MAX_GIAN_COEFFS = 10,
    NUM_STEPS = 5
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class LocoCrane254 : public LocoCrane
{
public:

    LocoCrane254(QObject *parent = Q_NULLPTR);

    ~LocoCrane254();

    double getHandlePosition() const;

    double getHandleShift() const;

    /// Получение номера позиции
    int getPositionNumber() const;

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

    double volume;

    double p_volume;

    int dir;

    int pos_num;

    bool isStop;

    std::array<double, NUM_STEPS> positions;

    std::array<double, NUM_STEPS> step_pressures;

    std::array<double, MAX_FLOW_COEFFS> K;

    std::array<double, MAX_GIAN_COEFFS> k;


    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);

    void stepSound();
};

#endif // KVT254_H
