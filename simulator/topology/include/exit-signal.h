#ifndef     EXIT_SIGNAL_H
#define     EXIT_SIGNAL_H

#include    <rail-signal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT ExitSignal : public Signal
{
public:

    ExitSignal(QObject *parent = Q_NULLPTR);

    ~ExitSignal();

    void step(double t, double dt) override;

private:

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;
};

#endif
