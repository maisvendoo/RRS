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

    // Освещение кабины тусклое
    processTumbler(CTRL_TUMBLER_CAB_LIGHT_LOW, pressed_keys);
    // Освещение кабины яркое
    processTumbler(CTRL_TUMBLER_CAB_LIGHT_HIGH, pressed_keys);
    // Освещение приборов
    processTumbler(CTRL_TUMBLER_LIGHT_DEVICES, pressed_keys);
    // Фонарь буферный левый
    processTumbler(CTRL_TUMBLER_BUF_LIGHT_L, pressed_keys);
    // Фонарь буферный правый
    processTumbler(CTRL_TUMBLER_BUF_LIGHT_R, pressed_keys);
    // Цвет буфера левый
    processTumbler(CTRL_TUMBLER_BUF_COLOR_L, pressed_keys);
    // Цвет буфера правый
    processTumbler(CTRL_TUMBLER_BUF_COLOR_R, pressed_keys);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_MODULE(VL60IOController)
