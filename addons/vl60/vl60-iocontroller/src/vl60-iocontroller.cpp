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
    if (pressed_keys.empty())
    {
        return;
    }

    // Управление токоприемниками
    processTumbler(CTRL_TUMBLER_PNT, pressed_keys);
    // Передний токоприемник
    processTumbler(CTRL_TUMBLER_PNT1, pressed_keys);
    // ЗАдний токоприемник
    processTumbler(CTRL_TUMBLER_PNT2, pressed_keys);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_MODULE(VL60IOController)
