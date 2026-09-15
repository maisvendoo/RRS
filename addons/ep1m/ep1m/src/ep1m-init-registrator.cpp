#include    "ep1m.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::initRegistartor(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;
    (void) custom_cfg_dir;

    registrator = new Registrator();
    registrator->setFileName("ep1m");
    registrator->read_config("registrator", custom_cfg_dir);
    registrator->init();
}
