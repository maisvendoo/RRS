#include    "passcar.h"

#include    "key-symbols.h"
#include    "coupling-operating-rod.h"
#include    "pneumo-anglecock.h"
#include    "pneumo-hose.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void PassCar::initControl(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;
    (void) custom_cfg_dir;

    // Концевые краны магистрали тормозной магистрали
    anglecock_bp_fwd->setKeySymbolOpen(KEY_F2);
    anglecock_bp_fwd->setKeyModifierOpen(MODIFIER_OnlyShift);
    anglecock_bp_fwd->setKeySymbolClose(KEY_F2);
    anglecock_bp_fwd->setKeyModifierClose(MODIFIER_OnlyControl);
    anglecock_bp_fwd->setControl(&pressed_keys);

    anglecock_bp_bwd->setKeySymbolOpen(KEY_F3);
    anglecock_bp_bwd->setKeyModifierOpen(MODIFIER_OnlyShift);
    anglecock_bp_bwd->setKeySymbolClose(KEY_F3);
    anglecock_bp_bwd->setKeyModifierClose(MODIFIER_OnlyControl);
    anglecock_bp_bwd->setControl(&pressed_keys);
}
