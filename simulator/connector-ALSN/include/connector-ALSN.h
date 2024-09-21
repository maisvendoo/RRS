#ifndef     CONNECTOR_ALSN_H
#define     CONNECTOR_ALSN_H

#include    "topology-connector-device.h"

//------------------------------------------------------------------------------
// Связи для рельсовых цепей АЛСН
//------------------------------------------------------------------------------
class ConnectorALSN : public ConnectorDevice
{
public:

    ConnectorALSN(QObject *parent = Q_NULLPTR);

    ~ConnectorALSN();

    TrajectoryDevice *getFwdTrajectoryDevice() const;
    TrajectoryDevice *getBwdTrajectoryDevice() const;

    void step(double t, double dt);

private:

    bool is_signal_fwd = false;
    bool is_signal_bwd = false;

    void load_config(CfgReader &cfg);
};

#endif // CONNECTOR_ALSN_H
