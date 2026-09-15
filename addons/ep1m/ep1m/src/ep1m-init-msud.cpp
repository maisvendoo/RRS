#include    "ep1m.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::initMSUD(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;

    msud = new MSUD();
    msud->read_config("msud", custom_cfg_dir);
    msud->init();
}
