#ifndef     KVT254_H
#define     KVT254_H

#include    "loco-crane.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    MAX_FLOW_COEFFS = 10,
    MAX_GIAN_COEFFS = 10
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

private:

    double V1;

    double V2;

    double Vpz;

    std::array<double, MAX_FLOW_COEFFS> K;

    std::array<double, MAX_GIAN_COEFFS> k;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // KVT254_H
