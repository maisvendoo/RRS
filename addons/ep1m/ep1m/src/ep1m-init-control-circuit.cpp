#include    "ep1m.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::initControlCircuit(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;

    kv44 = new Relay(4);
    kv44->read_config("mk-69", custom_cfg_dir);
    kv44->setInitContactState(0, false);
    kv44->setInitContactState(1, false);
    kv44->setInitContactState(2, false);
    kv44->setInitContactState(3, false);

    kv39 = new Relay(3);
    kv39->read_config("mk-69", custom_cfg_dir);
    kv39->setInitContactState(0, false);
    kv39->setInitContactState(1, false);
    kv39->setInitContactState(2, false);

    kv21 = new Relay(5);
    kv21->read_config("mk-69", custom_cfg_dir);
    kv21->setInitContactState(0, false);
    kv21->setInitContactState(1, false);
    kv21->setInitContactState(2, false);
    kv21->setInitContactState(3, true);
    kv21->setInitContactState(4, true);

    kv22 = new Relay(3);
    kv22->read_config("mk-69", custom_cfg_dir);
    kv22->setInitContactState(0, false);
    kv22->setInitContactState(1, false);
    kv22->setInitContactState(2, false);

    kv23 = new Relay(4);
    kv23->read_config("mk-69", custom_cfg_dir);
    kv23->setInitContactState(0, false);
    kv23->setInitContactState(1, false);
    kv23->setInitContactState(2, false);
    kv23->setInitContactState(3, false);

    kv41 = new Relay(4);
    kv41->read_config("mk-69", custom_cfg_dir);
    kv41->setInitContactState(0, false);
    kv41->setInitContactState(1, true);
    kv41->setInitContactState(2, false);
    kv41->setInitContactState(3, false);

    km43 = new Relay(4);
    km43->read_config("mk-69", custom_cfg_dir);
    km43->setInitContactState(0, false);
    km43->setInitContactState(1, false);
    km43->setInitContactState(2, false);
    km43->setInitContactState(3, false);

    kv11 = new Relay(4);
    kv11->read_config("mk-69", custom_cfg_dir);
    kv11->setInitContactState(0, false);
    kv11->setInitContactState(1, false);
    kv11->setInitContactState(2, false);
    kv11->setInitContactState(3, false);

    kv12 = new Relay(4);
    kv12->read_config("mk-69", custom_cfg_dir);
    kv12->setInitContactState(0, true);
    kv12->setInitContactState(1, false);
    kv12->setInitContactState(2, false);
    kv12->setInitContactState(3, false);

    kv13 = new Relay(4);
    kv13->read_config("mk-69", custom_cfg_dir);
    kv13->setInitContactState(0, false);
    kv13->setInitContactState(1, false);
    kv13->setInitContactState(2, false);
    kv13->setInitContactState(3, false);

    kv14 = new Relay(4);
    kv14->read_config("mk-69", custom_cfg_dir);
    kv14->setInitContactState(0, false);
    kv14->setInitContactState(1, false);
    kv14->setInitContactState(2, false);
    kv14->setInitContactState(3, false);

    kv15 = new Relay(6);
    kv15->read_config("mk-69", custom_cfg_dir);
    kv15->setInitContactState(0, false);
    kv15->setInitContactState(1, false);
    kv15->setInitContactState(2, false);
    kv15->setInitContactState(3, false);
    kv15->setInitContactState(4, false);
    kv15->setInitContactState(5, false);

    sp4 = new HysteresisRelay(0.27, 0.45);

    kv84 = new Relay(4);
    kv84->read_config("mk-69", custom_cfg_dir);
    kv84->setInitContactState(0, false);
    kv84->setInitContactState(1, false);
    kv84->setInitContactState(2, false);
    kv84->setInitContactState(3, false);

    kt10 = new TimeRelay(6);
    kt10->read_config("mk-69", custom_cfg_dir);
    kt10->setInitContactState(0, false);
    kt10->setInitContactState(1, false);
    kt10->setInitContactState(2, false);
    kt10->setInitContactState(3, false);
    kt10->setInitContactState(4, false);
    kt10->setInitContactState(5, false);
    kt10->setTimeoutOff(1.5);

    kt1 = new TimeRelay(5);
    kt1->read_config("mk-69", custom_cfg_dir);
    // Контакты цепи схемы "Тяга"
    kt1->setInitContactState(0, true);
    kt1->setInitContactState(1, false);
    kt1->setInitContactState(2, false);
    kt1->setInitContactState(3, false);
    // Контакты цепи схемы "Рекуперация"
    kt1->setInitContactState(4, true);
    kt1->setTimeoutOn(1.5);

    km41 = new Relay(5);
    km41->read_config("mk-69", custom_cfg_dir);
    km41->setInitContactState(0, false);
    km41->setInitContactState(1, false);
    km41->setInitContactState(2, false);
    km41->setInitContactState(3, false);
    km41->setInitContactState(4, true);

    km42 = new Relay(5);
    km42->read_config("mk-69", custom_cfg_dir);
    km42->setInitContactState(0, false);
    km42->setInitContactState(1, false);
    km42->setInitContactState(2, false);
    km42->setInitContactState(3, false);
    km42->setInitContactState(4, true);

    kt4 = new TimeRelay(5);
    kt4->read_config("mk-69", custom_cfg_dir);
    kt4->setInitContactState(0, true);
    kt4->setTimeoutOn(1.5);

    kt5 = new TimeRelay(5);
    kt5->read_config("mk-69", custom_cfg_dir);
    kt5->setInitContactState(0, false);
    kt5->setInitContactState(1, false);
    kt5->setTimeoutOff(2.0);

    km14 = new Relay(5);
    km14->read_config("mk-69", custom_cfg_dir);
    km14->setInitContactState(0, false);
    km14->setInitContactState(1, false);

    k1 = new Relay(5);
    k1->read_config("mk-69", custom_cfg_dir);
    k1->setInitContactState(0, false);
    k1->setInitContactState(1, false);
    k1->setInitContactState(2, true);

    Y3 = new PneumoElectroValve();
    Y3->read_config("epv", custom_cfg_dir);

    sp6 = new PressureSensor();
    sp6->read_config("sp6", custom_cfg_dir);

    Y4 = new PneumoElectroValve();
    Y4->read_config("epv", custom_cfg_dir);

    sp3 = new PressureSensor();
    sp3->read_config("sp3", custom_cfg_dir);

    Y5 = new PneumoElectroValve();
    Y5->read_config("epv", custom_cfg_dir);
}
