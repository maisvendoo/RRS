#include    "tep70.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::initElectroTransmission()
{
    // Возбудитель главного генератора
    field_gen = new FieldGenerator();

    kvv = new Relay(1);
    kvv->read_custom_config(config_dir + QDir::separator() + "mk-6");
    kvv->setInitContactState(0, false);

    kvg = new Relay(2);
    kvg->read_custom_config(config_dir + QDir::separator() + "mk-6");
    kvg->setInitContactState(0, false);
    kvg->setInitContactState(1, true);

    trac_gen = new TracGenerator();
    trac_gen->read_custom_config(config_dir + QDir::separator() + "trac-generator");
    trac_gen->load_marnetic_char(config_dir + QDir::separator() + "gs-504a.txt");
    trac_gen->load_eff_coeff(config_dir + QDir::separator() + "gs-504a-eff_coeff.txt");

    field_reg = new FieldRegulator();
    field_reg->read_custom_config(config_dir + QDir::separator() + "field-regulator");
    field_reg->load_settings(config_dir + QDir::separator() + "field-reg-settings.txt");

    for (size_t i = 0; i < motor.size(); ++i)
    {
        motor[i] = new TractionMotor();
        motor[i]->read_custom_config(config_dir + QDir::separator() + "ed-119");
        motor[i]->load_magnetic_char(config_dir + QDir::separator() + "ed-119.txt");
        motor[i]->load_eff_coeff(config_dir + QDir::separator() + "ed-119-eff_coeff.txt");

        kp[i] = new Relay(3);
        kp[i]->read_custom_config(config_dir + QDir::separator() + "mk-6");
        kp[i]->setInitContactState(0, false);
        kp[i]->setInitContactState(1, true);
        kp[i]->setInitContactState(2, false);

        kp[i]->setSoundName("Relay");
        connect(kp[i], &Relay::soundPlay, this, &TEP70::soundPlay);
    }

    kp[6] = new Relay(3);
    kp[6]->read_custom_config(config_dir + QDir::separator() + "mk-6");
    kp[6]->setInitContactState(0, false);
    kp[6]->setInitContactState(1, true);
    kp[6]->setInitContactState(2, false);

    speed_meter = new SL2M();
    speed_meter->read_custom_config(config_dir + QDir::separator() + "3SL-2M");

    ksh1 = new Relay(3);
    ksh1->read_custom_config(config_dir + QDir::separator() + "mk-6");
    ksh1->setInitContactState(0, false);
    ksh1->setInitContactState(1, false);
    ksh1->setInitContactState(2, false);

    ksh2 = new Relay(3);
    ksh2->read_custom_config(config_dir + QDir::separator() + "mk-6");
    ksh2->setInitContactState(0, false);
    ksh2->setInitContactState(1, false);
    ksh2->setInitContactState(2, true);

    ru1 = new Relay(3);
    ru1->read_custom_config(config_dir + QDir::separator() + "mk-6");
    ru1->setInitContactState(0, false);
    ru1->setInitContactState(1, false);
    ru1->setInitContactState(2, false);

    reversor = new Reversor();
    reversor->read_custom_config(config_dir + QDir::separator() + "reversor");

    brake_switcher = new BrakeSwitcher();
    brake_switcher->read_custom_config(config_dir + QDir::separator() + "brake-switcher");

    rp1 = new HysteresisRelay(0.079, 0.162);
    rp2 = new HysteresisRelay(0.079, 0.162);

    ksh2_delay = new TimeRelay(1);
    ksh2_delay->read_custom_config(config_dir + QDir::separator() + "rpu-3m");
    ksh2_delay->setTimeout(2.0);
    ksh2_delay->setInitContactState(0, false);

    ksh1_delay = new TimeRelay(1);
    ksh1_delay->read_custom_config(config_dir + QDir::separator() + "rpu-3m");
    ksh1_delay->setTimeout(2.0);
    ksh1_delay->setInitContactState(0, true);
}
