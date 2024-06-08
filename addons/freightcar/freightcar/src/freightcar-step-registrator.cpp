#include    "freightcar.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreightCar::stepRegistrator(double t, double dt)
{
    Q_UNUSED(t);
    Q_UNUSED(dt);
/*
    QString line = QString("%1 %2 %3 %4")
            .arg(t, 10, 'f', 1)
            .arg(brakepipe->getPressure(), 7, 'f', 3)
            .arg(supply_reservoir->getPressure(), 7, 'f', 3)
            .arg(brake_mech->getBCpressure(), 7, 'f', 3);

    registrator->print(line, t, dt);
*/
/*
    if ((t > 30.0) && (t < 180.0))
        registrator->print(air_dist->getDebugMsg(), t, dt);
*/
    QString line = QString("%1;%2;%3;%4")
                       .arg(t, 8, 'f', 3)
                       .arg(10.0*brakepipe->getPressure(), 11, 'f', 7)
                       .arg(10000.0*anglecock_bp_fwd->getFlowToPipe(), 12, 'f', 7)
                       .arg(anglecock_bp_fwd->getFlowCoeff(), 6, 'f', 4);
    registrator->print(line, t, dt);
}
