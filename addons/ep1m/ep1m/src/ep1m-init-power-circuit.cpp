#include    "ep1m.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::initPowerCircuit(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;

    for (size_t i = 0; i < pant.size(); ++i)
    {
        pant[i] = new Pantograph();
        pant[i]->read_config("pantograph", custom_cfg_dir);
    }

    main_switch = new ProtectiveDevice();

    trac_trans = new TractionTransformer();

    safety_valve = new ElectroValve();
    safety_valve->read_config("vz-6", custom_cfg_dir);


    for (size_t i = 0; i < trac_motor.size(); ++i)
    {
        trac_motor[i] = new TractionMotor();
        trac_motor[i]->read_config("nb-520v", custom_cfg_dir);
        trac_motor[i]->load_magnetic_char(custom_cfg_dir + QDir::separator() + "nb-520v.txt");
        trac_motor[i]->load_eff_coeff(custom_cfg_dir + QDir::separator() + "nb-520v-eff-coeff.txt");

        fast_switch[i] = new FastSwitch();
        fast_switch[i]->read_config("bv-8", custom_cfg_dir);
        fast_switch[i]->setInitContactState(0, false);
        fast_switch[i]->setInitContactState(1, true);
        fast_switch[i]->setInitContactState(2, true);
        fast_switch[i]->setInitContactState(3, false);
    }

    vip[VIP1] = new RectInvertConverter();
    vip[VIP1]->read_config("vip-5600", custom_cfg_dir);

    vip[VIP2] = new RectInvertConverter();
    vip[VIP2]->read_config("vip-5600", custom_cfg_dir);

    reversor = new Reversor();
    reversor->read_config("reversor", custom_cfg_dir);

    // Тормозной переключатель QT1
    qt1 = new BrakeSwitcher(10);
    qt1->read_config("pkd-15", custom_cfg_dir);
    qt1->setInitContactState(0, true);
    qt1->setInitContactState(1, true);
    qt1->setInitContactState(2, true);
    qt1->setInitContactState(3, true);
    qt1->setInitContactState(4, false);
    qt1->setInitContactState(5, false);
    qt1->setInitContactState(6, false);
    qt1->setInitContactState(7, true);
    qt1->setInitContactState(8, true);
    qt1->setInitContactState(9, true);

    // Шунты ослабления возбуждения
    shunts = new ShuntsModule();
    shunts->read_config("shunts-module", custom_cfg_dir);

    field_rect = new FieldRect();
}
