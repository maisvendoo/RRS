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

    kvg = new Relay(1);
    kvg->read_custom_config(config_dir + QDir::separator() + "mk-6");
    kvg->setInitContactState(0, false);

    trac_gen = new TracGenerator();
    trac_gen->read_custom_config(config_dir + QDir::separator() + "trac-generator");
    trac_gen->init(config_dir + QDir::separator() + "gs-504a.txt");

    field_reg = new FieldRegulator();
    field_reg->read_custom_config(config_dir + QDir::separator() + "field-regulator");
    field_reg->load_settings(config_dir + QDir::separator() + "field-reg-settings.txt");
}
