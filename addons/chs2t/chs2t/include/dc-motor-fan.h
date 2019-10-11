#ifndef DCMOTORFAN_H
#define DCMOTORFAN_H

#include    "device.h"
#include    "motor-magnetic-char.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DCMotorFan : public Device
{
public:

    DCMotorFan(QObject *parent = Q_NULLPTR);

    ~DCMotorFan();

private:

    double U;
    double Un;
    double In;
    double Nn;
    double Pn;
    double R;
    double E;
    double omega_nom;
    double omega;

    MotorMagneticChar cPhi;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    double calcCPhi(double I);
};

#endif // DCMOTORFAN_H
