#ifndef     PULSE_CONVERTER_H
#define     PULSE_CONVERTER_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DCPulseConverter : public Device
{
public:

    DCPulseConverter(QObject *parent = Q_NULLPTR);

    ~DCPulseConverter();

    double getU_out() const { return U_in * static_cast<double>(key1); }

    void setU_in(double U_in) { this->U_in = U_in; }

    void setRefCurrent(double I_ref) { this->I_ref = I_ref; }

    void setCurrent(double I) { this->I = I; }

private:

    bool key1;

    bool key2;

    double dI;

    double I_ref;

    double I;

    double U_in;

    void preStep(state_vector_t &Y, double t);    

    void ode_system(const state_vector_t &, state_vector_t &, double);

    void load_config(CfgReader &cfg);
};

#endif // PULSE_CONVERTER_H
