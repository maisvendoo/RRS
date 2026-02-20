#ifndef     CONNECTOR_ALSN_H
#define     CONNECTOR_ALSN_H

#include    "topology-connector-device.h"

//------------------------------------------------------------------------------
// Связи для рельсовых цепей АЛСН
//------------------------------------------------------------------------------
class ConnectorALSN : public ConnectorDevice
{
public:

    ConnectorALSN(QObject *parent = nullptr);

    ~ConnectorALSN();

    void step(double t, double dt);

private:

    void load_config(CfgReader &cfg);
};

#endif // CONNECTOR_ALSN_H
