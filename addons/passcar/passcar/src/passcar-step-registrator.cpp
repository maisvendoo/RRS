#include    "passcar.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCarrige::stepRegistrator(double t, double dt)
{
    if (reg == Q_NULLPTR)
        return;

    QString msg = QString("%1 %2 %3 ")
            .arg(hose_tm_fwd->getPressure(), 7, 'f', 5)
            .arg(brakepipe->getPressure(), 7, 'f', 5)
            .arg(hose_tm_bwd->getPressure(), 7, 'f', 5);
    if (next_vehicle == nullptr)
        msg += QString("%1\n").arg(t);

    reg->print_msg(msg, t, dt);
}
