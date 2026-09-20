#include    <key-symbols.h>
#include    <vl60-iocontroller.h>
#include    <vl60-controls.h>
#include    <core/get_module.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60IOController::VL60IOController() : IOController(nullptr)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60IOController::keysProcess(std::set<uint16_t> &pressed_keys)
{
    // Управление токоприемниками
    processTumbler(CTRL_TUMBLER_PNT, pressed_keys);
    // Передний токоприемник
    processTumbler(CTRL_TUMBLER_PNT1, pressed_keys);
    // Задний токоприемник
    processTumbler(CTRL_TUMBLER_PNT2, pressed_keys);

    // Включение ГВ
    processTumbler(CTRL_MAIN_SWITCH_ON, pressed_keys);
    // Возврат защиты ГВ
    processButton(CTRL_RETURN_PROTECTION, pressed_keys);

    // Фазорасщепитель
    processTumbler(CTRL_TUMBLER_FR, pressed_keys);

    // Компрессор
    processTumbler(CTRL_TUMBLER_MK, pressed_keys);

    // Вентиляторы
    processTumbler(CTRL_TUMBLER_MV1, pressed_keys);
    processTumbler(CTRL_TUMBLER_MV2, pressed_keys);
    processTumbler(CTRL_TUMBLER_MV3, pressed_keys);
    processTumbler(CTRL_TUMBLER_MV4, pressed_keys);
    processTumbler(CTRL_TUMBLER_MV5, pressed_keys);
    processTumbler(CTRL_TUMBLER_MV6, pressed_keys);

    // Цепи управления
    processTumbler(CTRL_TUMBLER_CU, pressed_keys);
    // Прожектор яркий
    processTumbler(CTRL_TUMBLER_SPOT_HIGH, pressed_keys);
    // Прожектор тусклый
    processTumbler(CTRL_TUMBLER_SPOT_LOW, pressed_keys);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_MODULE(VL60IOController)
