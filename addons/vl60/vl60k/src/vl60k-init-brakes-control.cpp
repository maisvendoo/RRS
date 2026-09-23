#include    "vl60k.h"
#include "vehicle-telemetry.h"
#include "brake-mech.h"
#include "dc-motor.h"

#include    <QDir>

#include "brake-crane.h"
#include "loco-crane.h"
#include "pneumo-brake-lock.h"
#include "pneumo-anglecock.h"
#include "pneumo-hose.h"
#include "pneumo-splitter.h"
#include "reservoir.h"
#include "core/load_module.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60k::initBrakesControl(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;

    for (size_t cab_idx : {CAB1, CAB2})
    {
        // Блокировочное устройство
        brake_lock[cab_idx] = new PneumoBrakeLock();
        brake_lock[cab_idx]->read_config("ubt367m");

        // Поездной кран машиниста
        brake_crane[cab_idx] = LOAD_MODULE(BrakeCrane,
            modules_dir + QDir::separator() + brake_crane_module_name);
        brake_crane[cab_idx]->read_config(brake_crane_config_name);

        // Кран вспомогательного тормоза
        loco_crane[cab_idx] = LOAD_MODULE(LocoCrane,
            modules_dir + QDir::separator() + loco_crane_module_name);
        loco_crane[cab_idx]->read_config(loco_crane_config_name);
    }

    // Телеметрия для кассеты регистрации и сессий (ТЗ "Кассеты",
    // п.2): адресация приборов живёт в VehicleTelemetry, а не в
    // Vehicle. УР общий на локомотив - кран I кабины, ТМ/ГР из
    // резервуаров, ТЦ - среднее по тележкам, ток - средний Ia моторов
    VehicleTelemetry::Sources telemetry;
    telemetry.equalizing_reservoir = [this]() -> double
    {
        return (brake_crane[CAB1] != nullptr)
                ? brake_crane[CAB1]->getERpressure()
                : -1.0;
    };
    telemetry.brake_pipe = [this]() -> double { return brakepipe->getPressure(); };
    telemetry.brake_cylinder = [this]() -> double
    {
        return 0.5 * (brake_mech[TROLLEY_FWD]->getBCpressure() +
                      brake_mech[TROLLEY_BWD]->getBCpressure());
    };
    telemetry.main_reservoir = [this]() -> double { return main_reservoir->getPressure(); };
    telemetry.traction_current = [this]() -> double
    {
        double sum = 0.0;
        int n = 0;
        for (const auto& m : motor)
        {
            if (m != nullptr)
            {
                sum += m->getIa();
                ++n;
            }
        }
        return (n > 0) ? sum / n : -1.0;
    };
    VehicleTelemetry::instance().bind(this, std::move(telemetry));

    // Импульсная магистраль с ложным тормозным цилиндром
    impulse_line = new Reservoir(0.005 + 0.007);

    // Тройник магистрали тормозных цилиндров
    bc_splitter = new PneumoSplitter();
    bc_splitter->read_config("bc-splitter", custom_cfg_dir);

    // Концевые краны магистрали тормозных цилиндров
    anglecock_bc_fwd = new PneumoAngleCock();
    anglecock_bc_fwd->read_config("pneumo-anglecock-BC");

    anglecock_bc_bwd = new PneumoAngleCock();
    anglecock_bc_bwd->read_config("pneumo-anglecock-BC");

    // Рукава магистрали тормозных цилиндров
    hose_bc_fwd = new PneumoHose();
    hose_bc_fwd->read_config("pneumo-hose-BC");
    forward_connectors.push_back(hose_bc_fwd);

    hose_bc_bwd = new PneumoHose();
    hose_bc_bwd->read_config("pneumo-hose-BC");
    backward_connectors.push_back(hose_bc_bwd);
}
