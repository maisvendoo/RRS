#ifndef     MSUT_H
#define     MSUT_H

#include    "device.h"

#include    "msut-data.h"
#include    "msut-derivative-calculator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MSUT : public Device
{
public:

    MSUT(QObject *parent = Q_NULLPTR);

    ~MSUT();

    void setInputData(const msut_input_t &msut_input)
    {
        this->msut_input = msut_input;
    }

    msut_output_t getOutputData() const
    {
        return msut_output;
    }

    void step(double t, double dt);

private:

    msut_input_t            msut_input;

    msut_output_t           msut_output;

    DerivativeCalculator    *accel_calc;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // MSUT_H
