#ifndef     ROUTE_SIGNAL_H
#define     ROUTE_SIGNAL_H

#include    <enter-signal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT RouteSignal : public EnterSignal
{
public:

    RouteSignal(QObject *parent = Q_NULLPTR);

    ~RouteSignal();

private:

    void preStep(state_vector_t &Y, double t) override;
};

#endif
