#ifndef     KRM_395_H
#define     KRM_395_H

#include    "brake-crane.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    MAX_FLOW_COEFFS = 14,
    NUM_POSITIONS = 7
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    EC_PRESSURE = 2
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    POS_I = 0,
    POS_II = 1,
    POS_III = 2,
    POS_IV = 3,
    POS_Va = 4,
    POS_V = 5,
    POS_VI = 6
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class BrakeCrane395 : public BrakeCrane
{
public:

    BrakeCrane395(QObject *parent = Q_NULLPTR);

    ~BrakeCrane395();

    void setPosition(int &position);

    QString getPositionName(int position);

    float getHandlePosition(int position);

private:

    double k_leek;

    double k_stab;

    double Vec;

    double A1;

    double k1;

    double k2;

    double k3;

    double T1;

    double T2;

    double K4_power;

    double Qbp;

    int old_input;

    int old_output;

    bool    pulse_II;

    DebugLog *debug_log;

    std::array<double, MAX_FLOW_COEFFS + 1> K;

    std::array<double, NUM_POSITIONS> pos;

    QStringList positions_names;

    std::vector<float> positions;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif
