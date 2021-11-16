#ifndef     ABSTRACT_SIGNAL_H
#define     ABSTRACT_SIGNAL_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    NUM_LENS = 5
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using lens_state_t = std::array<bool, NUM_LENS>;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Signal : public Device
{
public:

    Signal(QObject *parent = Q_NULLPTR);

    ~Signal();

    lens_state_t getAllLensState() const { return lens_state; };

protected:

     lens_state_t lens_state;

     virtual void preStep(state_vector_t &Y, double t);

     virtual void ode_system(const state_vector_t &Y,
                             state_vector_t &dYdt,
                             double t);


     virtual void load_config(CfgReader &cfg);
};

#endif // ABSTRACT_SIGNAL_H
