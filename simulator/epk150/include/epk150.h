#ifndef     EPK150_H
#define     EPK150_H

#include    "automatic-train-stop.h"

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
class AutoTrainStopEPK150 : public AutoTrainStop
{
public:

    AutoTrainStopEPK150(QObject *parent = Q_NULLPTR);

    ~AutoTrainStopEPK150();

private:

    std::array<double, MAX_FLOW_COEFFS> K;

    std::array<double, MAX_GIAN_COEFFS> k;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // EPK150_H
