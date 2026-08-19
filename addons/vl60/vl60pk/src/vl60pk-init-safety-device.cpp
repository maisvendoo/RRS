#include    <vl60pk.h>
#include    <QDir>

#include "ALSN-coil.h"
#include "ALSN-decoder.h"
#include "alsn-ukbm.h"
#include "automatic-train-stop.h"
#include "sl2m.h"
#include "speedmap.h"
#include "core/load_module.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initSafetyDevices(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;

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

    // Скоростемер
    for (auto i : {CAB1, CAB2})
    {
        speed_meter[i] = new SL2M();
        speed_meter[i]->read_config("3SL-2M", custom_cfg_dir);
    }
    speed_meter[CAB1]->setWheelDiameter(wheel_diameter[TED1]);
    speed_meter[CAB2]->setWheelDiameter(wheel_diameter[TED6]);

    for (size_t cab_idx : {CAB1, CAB2})
    {
        // ЭПК автостопа
        epk[cab_idx] = LOAD_MODULE(AutoTrainStop, modules_dir + QDir::separator() + "epk150");
        epk[cab_idx]->read_config("epk150");

        // Дешифратор АЛСН
        alsn_decoder[cab_idx] = new DecoderALSN();
        alsn_decoder[cab_idx]->read_config("ALSN-decoder");

        // УКБМ
        safety_device[cab_idx] = new SafetyDevice;
    }
}
