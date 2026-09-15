#include    "ep1m.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::initPanel(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;

    for (size_t cab_idx : {CAB1, CAB2})
    {
        tumblers_panel[cab_idx] = new EP1MTumblersPanel();

        km[cab_idx] = new TracController();
        km[cab_idx]->read_config("km-35-01", custom_cfg_dir);
    }

    signals_module = new SignalizationModule();
}
