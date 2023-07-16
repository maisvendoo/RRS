#include    "passcar.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCar::initRegistrator()
{
    registrator = new Registrator();
    registrator->read_custom_config(config_dir + QDir::separator() + "registrator");
}
