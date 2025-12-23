#include    "passcar.h"

#include    "key-symbols.h"
#include    "coupling-operating-rod.h"
#include    "pneumo-anglecock.h"
#include    "pneumo-hose-epb.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void PassCar::initControl(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;
    (void) custom_cfg_dir;

    // Включение трёх красных огней на передней торцевой стенке
    red_lamps_end_of_train_fwd.setKeySymbolOn(KEY_G);
    red_lamps_end_of_train_fwd.setKeyModifierOn(MODIFIER_OnlyShift);
    red_lamps_end_of_train_fwd.setKeySymbolOff(KEY_G);
    red_lamps_end_of_train_fwd.setKeyModifierOff(MODIFIER_OnlyControl);
    red_lamps_end_of_train_fwd.setControl(&pressed_keys);

    // Включение трёх красных огней на задней торцевой стенке
    red_lamps_end_of_train_bwd.setKeySymbolOn(KEY_J);
    red_lamps_end_of_train_bwd.setKeyModifierOn(MODIFIER_OnlyShift);
    red_lamps_end_of_train_bwd.setKeySymbolOff(KEY_J);
    red_lamps_end_of_train_bwd.setKeyModifierOff(MODIFIER_OnlyControl);
    red_lamps_end_of_train_bwd.setControl(&pressed_keys);

    // Расцепные рычаги
    oper_rod_fwd->setKeySymbol(KEY_X);
    oper_rod_fwd->setControl(&pressed_keys);

    oper_rod_bwd->setKeySymbol(KEY_C);
    oper_rod_bwd->setControl(&pressed_keys);

    // Концевые краны тормозной магистрали
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

    // Рукава тормозной магистрали
    hose_bp_fwd->setKeySymbolConnect(KEY_F1);
    hose_bp_fwd->setKeyModifierConnect(MODIFIER_OnlyShift);
    hose_bp_fwd->setKeySymbolDisconnect(KEY_F1);
    hose_bp_fwd->setKeyModifierDisconnect(MODIFIER_OnlyControl);
    hose_bp_fwd->setControl(&pressed_keys);

    hose_bp_bwd->setKeySymbolConnect(KEY_F4);
    hose_bp_bwd->setKeyModifierConnect(MODIFIER_OnlyShift);
    hose_bp_bwd->setKeySymbolDisconnect(KEY_F4);
    hose_bp_bwd->setKeyModifierDisconnect(MODIFIER_OnlyControl);
    hose_bp_bwd->setControl(&pressed_keys);

    // Управление башмаками
    brake_shoes_set.setKeySymbolOn(KEY_Insert);
    brake_shoes_set.setKeyModifierOn(MODIFIER_OnlyShift);
    brake_shoes_set.setKeySymbolOff(KEY_Insert);
    brake_shoes_set.setKeyModifierOff(MODIFIER_OnlyControl);
    brake_shoes_set.setControl(&pressed_keys);
}
