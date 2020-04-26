#ifndef     BRAKE_SWITCHER_H
#define     BRAKE_SWITCHER_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class BrakeSwitcher : public Device
{
public:

    BrakeSwitcher(QObject *parent = Q_NULLPTR);

    ~BrakeSwitcher();

    void setTracValeState(bool trac_valve) { this->trac_valve = trac_valve; }

    void setBrakeValveState(bool brake_valve) { this->brake_valve = brake_valve; }

    int getState() const { return state; }

    bool isTraction() const { return state == 1; }

    bool isBrake() const { return state == -1; }

private:

    int     state;

    bool    trac_valve;

    bool    brake_valve;

    /// Максимальная скорость поворота вала
    double  shaft_omega_max;

    /// Фактическая скорость поворота вала
    double  omega;

    double  pos_ref;

    double  K;

    double  eps;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // BRAKE_SWITCHER_H
