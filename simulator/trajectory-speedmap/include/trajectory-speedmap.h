#ifndef     TRAJECTORY_SPEEDMAP_H
#define     TRAJECTORY_SPEEDMAP_H

#include    "topology-trajectory-device.h"

//------------------------------------------------------------------------------
// Автосцепка са-3
//------------------------------------------------------------------------------
class TrajectorySpeedMap : public TrajectoryDevice
{
public:

    TrajectorySpeedMap(QObject *parent = Q_NULLPTR);

    ~TrajectorySpeedMap();

    virtual void step(double t, double dt);

private:

    void load_config(CfgReader &cfg);
};

#endif // TRAJECTORY_SPEEDMAP_H
