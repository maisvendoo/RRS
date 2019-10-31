//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------

#ifndef     EVR305_H
#define     EVR305_H

const int MAX_COEFFS = 6;
const int MAX_LCOEFFS = 2;

#include    "electro-airdistributor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EVR305 : public ElectroAirDistributor
{
public:

    EVR305(QObject *parent = Q_NULLPTR);

    ~EVR305();

    void setPzpk_in(double value) { pzpk_in = value; }

private:

    int a;
    double A1;
    double Vpk;
    double pzpk_in;
    double ptc;

    std::array<double, MAX_COEFFS> K;
    std::array<double, MAX_LCOEFFS> k;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void preStep(state_vector_t &Y, double t);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);
};

#endif // EPK150_H
