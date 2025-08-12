#include    <vl60k.h>
#include    <QDir>

#include "ALSN-coil.h"
#include "ALSN-decoder.h"
#include "automatic-train-stop.h"
#include "alsn-ukbm.h"
#include "sl2m.h"
#include "speedmap.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::initSafetyDevices(const QString &modules_dir, const QString &custom_cfg_dir)
{
    (void) modules_dir;

    // Карта ограничений скорости
    speedmap_fwd = new SpeedMap();
    speedmap_fwd->setDirection(dir * orient);
    addRailwayConnector(speedmap_fwd, length / 2.0);

    speedmap_bwd = new SpeedMap();
    speedmap_bwd->setDirection(-1 * dir * orient);
    addRailwayConnector(speedmap_bwd, -length / 2.0);

    // Приёмные катушки АЛСН
    coil_ALSN_fwd = new CoilALSN();
    coil_ALSN_fwd->setDirection(dir * orient);
    addRailwayConnector(coil_ALSN_fwd, length / 2.0);

    coil_ALSN_bwd = new CoilALSN();
    coil_ALSN_bwd->setDirection(-1 * dir * orient);
    addRailwayConnector(coil_ALSN_bwd, -length / 2.0);

    // Скоростемер
    speed_meter[CAB1] = new SL2M();
    speed_meter[CAB1]->setWheelDiameter(wheel_diameter[0]);
    speed_meter[CAB1]->read_config("3SL-2M", custom_cfg_dir);

    speed_meter[CAB2] = new SL2M();
    speed_meter[CAB2]->setWheelDiameter(wheel_diameter[0]);
    speed_meter[CAB2]->read_config("3SL-2M", custom_cfg_dir);

    for (size_t cab_idx : {CAB1, CAB2})
    {
        // ЭПК автостопа
        epk[cab_idx] = loadAutoTrainStop(modules_dir + QDir::separator() + "epk150");
        epk[cab_idx]->read_config("epk150");

        // Дешифратор АЛСН
        alsn_decoder[cab_idx] = new DecoderALSN();
        alsn_decoder[cab_idx]->read_config("ALSN-decoder");

        // УКБМ
        safety_device[cab_idx] = new SafetyDevice;
    }
}
