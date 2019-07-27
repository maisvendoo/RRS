#ifndef     RECTIFIER_H
#define     RECTIFIER_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Rectifier : public Device
{
public:

    Rectifier(QObject *parent = Q_NULLPTR);

    ~Rectifier();

    void setU_in(double value);

    double getU_out() const;

    void setI_out(double value);

private:

    /// Коэффициент, связывающией входное и выходное напряжение
    double coeff;

    double U_in;

    double I_out;

    double U_out;

    double r;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // RECTIFIER_H
