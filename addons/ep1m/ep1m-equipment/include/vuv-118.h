#ifndef     VUV118_H
#define     VUV118_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FieldRect : public Device
{
public:

    FieldRect(QObject *parent = Q_NULLPTR);

    ~FieldRect();

    double getOutputVoltage() const
    {
        return U_out;
    }

    void setInputVoltage(double U_in)
    {
        this->U_in = U_in;
    }

    void setVoltageLevel(double voltage_level)
    {
        this->voltage_level = cut(voltage_level, 0.0, 1.0);
    }

private:

    double U_in;

    double U_out;

    double voltage_level;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // VUV118_H
