#include    <key-symbols.h>
#include    <vl60-iocontroller.h>
#include    <vl60-controls.h>
#include    <core/get_module.h>

#include    <km-state.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60IOController::VL60IOController() : IOController(nullptr)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60IOController::init()
{
    IOController::init();

    // Получаем обработчки, связанные с контроллером машиниста
    for (auto cab_idx : {CAB1, CAB2})
    {
        auto handler = control_handlers[cab_idx].getByKey1(CTRL_REVERS_INSERTION);
        revers_handle_holder[cab_idx] = handler.value();

        handler = control_handlers[cab_idx].getByKey1(CTRL_REVERS_POSITION);
        revers_handle[cab_idx] = handler.value();

        handler = control_handlers[cab_idx].getByKey1(CTRL_KM_MAIN_POSITION);
        main_handle[cab_idx] = handler.value();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60IOController::step(float t, float dt, const std::vector<float> *server_signals)
{
    IOController::step(t, dt, server_signals);

    for (auto cab_idx : {CAB1, CAB2})
    {
        lockReversHandle(cab_idx);
        lockMainHandle(cab_idx);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60IOController::processKeyboardInput(std::set<uint16_t> &pressed_keys)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60IOController::lockReversHandle(int cab_idx)
{
    int main_pos = main_handle[cab_idx]->getSignalValue();
    int revers_pos = revers_handle[cab_idx]->getSignalValue();

    float ref_pos = revers_handle[cab_idx]->value;

    if (main_pos != POS_ZERO)
    {
        if (revers_pos == REVERS_BACKWARD)
        {
            revers_handle[cab_idx]->value = revers_pos;
            revers_handle[cab_idx]->sendControlSignal();
            return;
        }

        if ((revers_pos >= REVERS_FORWARD) && (ref_pos < REVERS_FORWARD))
        {
            revers_handle[cab_idx]->value = revers_pos;
            revers_handle[cab_idx]->sendControlSignal();
            return;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60IOController::lockMainHandle(int cab_idx)
{
    int revers_pos = revers_handle[cab_idx]->getSignalValue();

    if (revers_pos == REVERS_ZERO)
    {
        main_handle[cab_idx]->value = POS_ZERO;
        main_handle[cab_idx]->sendControlSignal();
        return;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_MODULE(VL60IOController)
