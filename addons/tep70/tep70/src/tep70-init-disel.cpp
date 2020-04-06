#include    "tep70.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::initDisel()
{
    disel = new Disel();
    disel->read_custom_config(config_dir + QDir::separator() + "disel");

    starter_generator = new StarterGenerator();
    starter_generator->read_custom_config(config_dir + QDir::separator() + "starter-generator");
    starter_generator->init(config_dir + QDir::separator() + "5sg-magnetic-char.txt");

    voltage_regulator = new VoltageRegulator();
    voltage_regulator->read_custom_config(config_dir + QDir::separator() + "voltage-regulator");
}
