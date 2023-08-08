#include    "filesystem.h"

#include    "chs2t.h"

#include    "Journal.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void CHS2T::initBrakesControl(QString modules_dir)
{
    //Journal::instance()->info("Init brake control devices");

    // Поездной кран машиниста
    brake_crane = loadBrakeCrane(
                modules_dir + QDir::separator() + brake_crane_module_name);
    brake_crane->read_config(brake_crane_config_name);
    connect(brake_crane, &BrakeCrane::soundPlay, this, &CHS2T::soundPlay);
    connect(brake_crane, &BrakeCrane::soundSetVolume, this, &CHS2T::soundSetVolume);

    // Кран вспомогательного тормоза
    loco_crane = loadLocoCrane(
                modules_dir + QDir::separator() + loco_crane_module_name);
    loco_crane->read_config(loco_crane_config_name);
    connect(loco_crane, &LocoCrane::soundPlay, this, &CHS2T::soundPlay);
    connect(loco_crane, &LocoCrane::soundStop, this, &CHS2T::soundStop);
    connect(loco_crane, &LocoCrane::soundSetVolume, this, &CHS2T::soundSetVolume);

    // Рукоятка задатчика тормозного усилия
    handleEDT = new HandleEDT();
    handleEDT->read_custom_config(config_dir + QDir::separator() + "handle-edt");
    handleEDT->setBrakeKey(KEY_Period);
    handleEDT->setReleaseKey(KEY_Comma);

    // Электропневматический клапан автостопа
    epk = loadAutoTrainStop(modules_dir + QDir::separator() + "epk150");
    epk->read_config("epk150");
    connect(epk, &AutoTrainStop::soundPlay, this, &CHS2T::soundPlay);
    connect(epk, &AutoTrainStop::soundStop, this, &CHS2T::soundStop);

    // Электропневматический вентиль экстренного торможения
    emergency_valve = new ElectroPneumoValveEmergency();
    emergency_valve->read_custom_config(config_dir + QDir::separator() + "valve-emergency");

    // Электропневматический вентиль отпуска тормозов
    release_valve = new ElectroPneumoValveRelease();
    release_valve->read_custom_config(config_dir + QDir::separator() + "valve-release");

    // Управляющая камера воздухораспределителя (ложный ТЦ)
    brake_ref_res = new Reservoir(0.01);

    // Разветвитель потока воздуха от локомотивного крана к тележкам
    loco_crane_splitter = new PneumoSplitter();
    loco_crane_splitter->read_config("pneumo-splitter");

    // Скоростной клапан ДАКО
    dako = new Dako();
    dako->setWheelRadius(rk[5]);
    dako->read_custom_config(config_dir + QDir::separator() + "dako");

    // Переключательный клапан магистрали тормозных цилиндров
    bc_switch_valve[TROLLEY_FWD] = new SwitchingValve();
//    bc_switch_valve[TROLLEY_FWD]->read_custom_config(config_dir + QDir::separator() + "zpk");
    bc_switch_valve[TROLLEY_FWD]->read_config("zpk");
    bc_switch_valve[TROLLEY_BWD] = new SwitchingValve();
//    bc_switch_valve[TROLLEY_BWD]->read_custom_config(config_dir + QDir::separator() + "zpk");
    bc_switch_valve[TROLLEY_BWD]->read_config("zpk");

    // Повторительное реле давления
    bc_pressure_relay = new PneumoRelay();
    bc_pressure_relay->read_config("rd304");
}
