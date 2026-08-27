#ifndef     SL2M_H
#define     SL2M_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SL2M : public Device
{
public:

    SL2M(QObject *parent = nullptr);

    ~SL2M();

    void setOmega(double value);

    void setWheelDiameter(double diam);

    float getArrowPos() const;

    float getShaftPos() const;

    virtual sound_state_t getSoundState(size_t idx = 0) const;

    virtual float getSoundSignal(size_t idx = 0) const;

    double getVelocity() const
    {
        return velocity;
    }

private:

    double omega;

    double ip;

    double max_speed;

    float arrow_pos;

    double Dk;

    double speed_begin_sound;
    double omega_begin_sound;

    float shaft_pos;

    double velocity = 0.0;

    double seg_height[3];
    int    lifting_idx;
    int    holding_idx;
    int    falling_idx;

    double cycle_time;
    double phase;
    double t_prev;

    double gear_wear;
    double holding_slip;
    double clock_jitter;
    double fall_speed;

    double phase_velocity;

    double shaft_angle;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // SL2M_H