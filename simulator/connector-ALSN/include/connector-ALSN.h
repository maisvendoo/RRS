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

    void step(double t, double dt) override;

private:

    void load_config(CfgReader &cfg) override;
};

#endif // CONNECTOR_ALSN_H
