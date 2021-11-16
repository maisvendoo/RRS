#ifndef     LINE_SIGNAL_H
#define     LINE_SIGNAL_H

#include    "abstract-signal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class LineSignal : public Signal
{
public:

    LineSignal(QObject *parent = Q_NULLPTR);

    ~LineSignal();

private:

    void preStep(state_vector_t &Y, double t);

    void load_config(CfgReader &cfg);
};

#endif // LINE_SIGNAL_H
