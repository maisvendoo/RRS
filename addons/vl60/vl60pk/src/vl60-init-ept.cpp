#include    "vl60.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initEPT(const QString &modules_dir)
{
    // Инициализация линии управления
    ept_control.resize(1);
    ept_current.resize(1);

    ept_control[0] = ept_current[0] = 0.0;

    // Инициализация источника питания
    ept_converter = new EPTConverter();
    ept_converter->read_config("ept-converter");

    // Инциализация блока управления
    ept_pass_control = new EPTPassControl();

    // Инициализация ЭВР 305
    electroAirDist = loadElectroAirDistributor(modules_dir + QDir::separator() + "evr305");
    electroAirDist->read_config("evr305");
}
