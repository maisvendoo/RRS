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
    for (auto cab_idx : {CAB1, CAB2})
    {
        // Обнуляем положение реверса, если нет реверсивки
        if (!revers_handle_holder[cab_idx]->toBool())
        {
            revers_handle[cab_idx]->value = REVERS_ZERO;
        }

        // Обнуляем положение главной рукоятки при нулевом положении реверса
        if (revers_handle[cab_idx]->toInt() == REVERS_ZERO)
        {
            main_handle[cab_idx]->value = POS_ZERO;
        }
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
GET_MODULE(VL60IOController)
