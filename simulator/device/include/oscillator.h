#ifndef     OSCILLATOR_H
#define     OSCILLATOR_H

#include    "device.h"

class Oscillator : public Device
{
public:

    Oscillator(QObject *parent);

    ~Oscillator();

    double getOutput() const;

private:

    double      omega;
    double      beta;

    double      input;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // OSCILLATOR_H
