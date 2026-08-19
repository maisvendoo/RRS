#include    "connector-speedmap.h"

#include    <core/get_module.h>

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
void ConnectorSpeedMap::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getString(secName, "Name", name);
}

GET_MODULE(ConnectorSpeedMap)
