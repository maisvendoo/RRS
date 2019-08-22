#ifndef     PNEUMO_SPLITTER_H
#define     PNEUMO_SPLITTER_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoSplitter : public BrakeDevice
{
public:

    PneumoSplitter(QObject *parent = Q_NULLPTR);

    ~PneumoSplitter();

    void setQ_in(double value);

    void setP_out1(double value);

    void setP_out2(double value);

    double getQ_out1() const;

    double getQ_out2() const;

    double getP_in() const;

private:

    double V0;

    double Q_in;

    double Q_out1;

    double p_out1;

    double Q_out2;

    double p_out2;

    enum
    {
        NUM_COEFFS = 4
    };

    std::array<double, NUM_COEFFS> K;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // PNEUMO_SPLITTER_H
