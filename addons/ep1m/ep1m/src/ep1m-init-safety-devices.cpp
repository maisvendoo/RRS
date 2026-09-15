#include    "ep1m.h"

#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::initSafetyDevices(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;
    (void) custom_cfg_dir;

    // Карта ограничений скорости
    speedmap_fwd = new SpeedMap();
    speedmap_fwd->setDirection(1);
    addRailwayConnector(speedmap_fwd, length / 2.0);

    speedmap_bwd = new SpeedMap();
    speedmap_bwd->setDirection(-1);
    addRailwayConnector(speedmap_bwd, -length / 2.0);

    // Приёмные катушки АЛСН
    coil_ALSN_fwd = new CoilALSN();
    coil_ALSN_fwd->setDirection(1);
    addRailwayConnector(coil_ALSN_fwd, length / 2.0);

    coil_ALSN_bwd = new CoilALSN();
    coil_ALSN_bwd->setDirection(-1);
    addRailwayConnector(coil_ALSN_bwd, -length / 2.0);

    // Дешифратор АЛСН
    for (size_t cab_idx : {CAB1, CAB2})
    {
        alsn_decoder[cab_idx] = new DecoderALSN();
        alsn_decoder[cab_idx]->read_config("ALSN-decoder");
    }

    // КЛУБ
    klub_BEL = new KLUB();
    klub_BEL->setMaxVelocity(140.0);
    klub_BEL->setSpeedMapModule(speedmap_fwd);
    klub_BEL->setCoilALSNModule(coil_ALSN_fwd);
    klub_BEL->setTrainLength(length);

    // Загрузка станций в КЛУБ
    FileSystem &fs = FileSystem::getInstance();
    QString path = fs.getRouteRootDir().c_str();
    path += QDir::separator() + route_dir;
    path += QDir::separator() + QString("topology");
    path += QDir::separator() + QString("stations.conf");
    klub_BEL->loadStationsMap(path);
}
