//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------

#ifndef     EVR305_H
#define     EVR305_H

const int MAX_COEFFS = 7;
const int MAX_LCOEFFS = 3;

#include    "electro-airdistributor.h"
#include    "switching-valve.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EVR305 : public ElectroAirDistributor
{
public:

    EVR305(QObject *parent = Q_NULLPTR);

    ~EVR305();

private:
    std::array<double, MAX_COEFFS> K;
    std::array<double, MAX_LCOEFFS> k;

    SwitchingValve *zpk;

    double A1;
    double Vpk;
    double Q1;

    double Ip_max;
    double It_max;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void preStep(state_vector_t &Y, double t);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);

    void step(double t, double dt);
};

#endif // EPK150_H
