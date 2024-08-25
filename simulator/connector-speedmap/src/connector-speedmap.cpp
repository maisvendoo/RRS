#include    "connector-speedmap.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ConnectorSpeedMap::ConnectorSpeedMap(QObject *parent) : ConnectorDevice(parent)
{
    name = QString("speedmap");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ConnectorSpeedMap::~ConnectorSpeedMap()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConnectorSpeedMap::step(double t, double dt)
{
    (void) t;
    (void) dt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConnectorSpeedMap::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getString(secName, "Name", name);
}

GET_CONNECTOR_DEVICE(ConnectorSpeedMap)
