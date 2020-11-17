#ifndef     HYSTERESIS_RELAY_H
#define     HYSTERESIS_RELAY_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class HysteresisRelay : public Device
{
public:

    HysteresisRelay(double x_min = 0.0,
                    double x_max = 0.0, QObject *parent = Q_NULLPTR);

    virtual ~HysteresisRelay();

    void setInput(double x)
    {
        this->x_prev = this->x;
        this->x = x;
    }

    int getOutput() const {return state; }

    void setActive(bool is_active) { this->is_active = is_active; }

    void setLocked(bool is_locked) { this->is_locked = is_locked; }

protected:

    double  x;

    double  x_prev;

    double  x_min;

    double  x_max;

    int     state;

    bool    is_active;

    bool    is_locked;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // HYSTERESIS_RELAY_H
