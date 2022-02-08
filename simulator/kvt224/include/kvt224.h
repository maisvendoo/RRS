#ifndef KVT224_H
#define KVT224_H

#include    "loco-crane.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    MAX_FLOW_COEFFS = 10,
    MAX_GIAN_COEFFS = 10,
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

    double getHandlePosition() const;

    double getHandleShift() const;

    double getAirDistribPressure() const;

    void init(double pTM, double pFL);

private:

    double V1;

    double V2;

    double Vpz;

    double delta_p;

    double ps;

    double min_pos;

    double max_pos;

    double pos_duration;

    int dir;


    std::array<double, NUM_STEPS> step_pressures;

    std::array<double, NUM_STEPS> fixed_pos;

    std::array<double, MAX_FLOW_COEFFS> K;

    std::array<double, MAX_GIAN_COEFFS> k;

    /// Текущая фиксированная позиция
    double cur_pos;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);
};

#endif // KVT224_H
