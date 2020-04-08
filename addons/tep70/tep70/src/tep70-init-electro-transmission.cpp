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
    kvv->read_custom_config(config_dir + QDir::separator() + "rpu-8m");
    kvv->setInitContactState(0, false);

    kvg = new Relay(1);
    kvg->read_custom_config(config_dir + QDir::separator() + "rpu-8m");
    kvg->setInitContactState(0, false);

    trac_gen = new TracGenerator();
    trac_gen->read_custom_config(config_dir + QDir::separator() + "trac-generator");

    field_reg = new FieldRegulator();
    field_reg->read_custom_config(config_dir + QDir::separator() + "field-regulator");
    field_reg->load_settings(config_dir + QDir::separator() + "field-reg-settings.txt");
}
