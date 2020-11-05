#ifndef     ENERGY_COUNTER_H
#define     ENERGY_COUNTER_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class   EnergyCounter : public Device
{
public:

    EnergyCounter(QObject *parent = Q_NULLPTR);

    ~EnergyCounter();

    void setFullPower(double P_ks) { this->P_ks = P_ks; }

    void setResistorsPower(double P_ptr) { this->P_ptr = P_ptr; }

    double getFullEnergy() const { return getY(0); }

    double getResistorEnergy() const { return getY(1); }

    double getTracEnergy() const { return getY(2); }

    double getFullPower() const { return P_ks / 1000.0; }

    double getResistorPower() const { return P_ptr / 1000.0; }

    double getTracPower() const { return P_trac / 1000.0; }

private:

    /// Мощность, потребляемая из КС
    double P_ks;

    /// Мощность, сжигаемая на ПТР
    double P_ptr;

    /// Мощность, идущая на образование силы тяги
    double P_trac;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void preStep(state_vector_t &Y, double t);

    void load_config(CfgReader &cfg);
};

#endif // ENERGY_COUNTER_H
