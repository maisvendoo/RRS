#ifndef     CONNECTOR_SPEEDMAP_H
#define     CONNECTOR_SPEEDMAP_H

#include    "topology-connector-device.h"

//------------------------------------------------------------------------------
// Связи для карты ограничений скорости
//------------------------------------------------------------------------------
class ConnectorSpeedMap : public ConnectorDevice
{
public:

    ConnectorSpeedMap(QObject *parent = nullptr);

    ~ConnectorSpeedMap();

private:

    void load_config(CfgReader &cfg) override;
};

#endif // CONNECTOR_SPEEDMAP_H
