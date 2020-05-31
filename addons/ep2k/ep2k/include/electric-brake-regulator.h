#ifndef     ELECTRIC_BRAKE_REGULATOR_H
#define     ELECTRIC_BRAKE_REGULATOR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EDBRegulator : public Device
{
public:

    EDBRegulator(QObject *parent = Q_NULLPTR);

    ~EDBRegulator();

    void setBrakeLevel(double brake_level)
    {
        this->brake_level = cut(brake_level, -1.0, 0.0);
    }

    void setAncorCurrent(double Ia) { this->Ia = Ia; }

    void setOmega(double omega) { this->omega = omega; }

    double getRefAncorCurrent() const { return Ia_ref; }

    double getRefFieldCurrent() const {return If_ref; }

private:

    double I_max;

    double K1;

    double K2;

    double dI;

    double brake_level;

    double Ia;

    double If_ref;

    double Ia_ref;

    double omega_nom;

    double omega;

    double omega_min;


    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &, state_vector_t &, double);

    void load_config(CfgReader &cfg);
};

#endif // ELECTRIC_BRAKE_REGULATOR_H
