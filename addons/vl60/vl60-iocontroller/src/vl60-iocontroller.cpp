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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_MODULE(VL60IOController)
