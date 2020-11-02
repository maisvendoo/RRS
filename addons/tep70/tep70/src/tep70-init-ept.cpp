#include    "tep70.h"

#include    <QDir>

void TEP70::initEPT()
{
    // Инициализация линии управления ЭПТ
    ept_control.resize(1);
    ept_current.resize(1);

    ept_control[0] = ept_current[0];

    // Инициализайия источника питания ЭПТ
    ept_converter = new EPTConverter();
    ept_converter->read_custom_config(config_dir + QDir::separator() + "ept-converter");

    // Инициализация блока управления ЭПТ (ПОЗЖЕ!!!)
    ept_pass_control = new EPTPassControl();
}
