#include    <io-controller.h>
#include    <io-controller-keymap.h>
#include    <CfgReader.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
IOController::IOController(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::setPressedKey(uint16_t keyBase)
{
    if (KeySymbolsRRS.count(keyBase))
    {
        auto result = _pressed_keys.insert(keyBase);
        if (result.second)
        {
            processControl(CTRL_TYPE_KEYBOARD);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::setReleasedKey(uint16_t keyBase)
{
    _pressed_keys.erase(keyBase);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::step(float t, float dt)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool IOController::load_config(CfgReader &cfg)
{
    auto secNode = cfg.getFirstSection("Control");

    while (!secNode.isNull())
    {
        io_control_input_t ic_input;

        int control_ID = 0;
        cfg.getInt(secNode, "ID", control_ID);
        ic_input.id = static_cast<uint16_t>(control_ID);

        double value = 0.0;
        cfg.getDouble(secNode, "value", value);
        ic_input.value = static_cast<float>(value);

        QString keyName = "";
        cfg.getString(secNode, "KeyName", keyName);
        ic_input.keyCode = KeySymbolsRRSMap.value(keyName, KEY_Undefined);

        cfg.getString(secNode, "ObjectName", ic_input.contolledObjectName);

        io_control_inputs.insert(ic_input.id, ic_input.contolledObjectName, ic_input);

        secNode = cfg.getNextSection();
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::setCabineIndex(int vehicle_idx, int cab_idx)
{
    for (auto &[key1, key2, value] : io_control_inputs.getAll())
    {
        value.controlled_vehicle_idx = vehicle_idx;
        value.cabine_idx = cab_idx;

        io_control_inputs.updateByKey1(key1, value);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::keysProcess(std::set<uint16_t> &pressed_keys)
{
    (void) pressed_keys;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::processTumbler(const uint16_t &control_id,
                                  const std::set<uint16_t> &pressed_keys)
{
    // Проверяем конкретный контрол
    auto io_ctrl = io_control_inputs.getByKey1(control_id);

    // Нажата ли его клавиша
    if (getKeyState(pressed_keys, io_ctrl->keyCode))
    {
        // Какой модификатор?
        if (isShift(pressed_keys))
        {
            io_ctrl->value = 1.0f;
            emit sigSendVehicleControlCommand(io_ctrl->serialize());
            return;
        }

        if (isControl(pressed_keys))
        {
            io_ctrl->value = 0.0f;
            emit sigSendVehicleControlCommand(io_ctrl->serialize());
            return;
        }        
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::processKeyBoardInput()
{
    // Уходим, если ничего не нажато
    if (_pressed_keys.empty())
    {
        return;
    }

    // Если массив нажатых клавиш содержит только Shift, Ctrl, Alt
    // отправляем пустое управление
    constexpr KeySymbol modifier_keys[] = {KEY_Shift_L, KEY_Shift_R, KEY_Control_L, KEY_Control_R, KEY_Alt_L, KEY_Alt_R};
    std::size_t modifiers_size = 0;
    for (std::uint16_t key : modifier_keys)
    {
        if (_pressed_keys.count(key))
        {
            ++modifiers_size;
        }
    }

    if (_pressed_keys.size() == modifiers_size)
    {
        return;
    }

    std::set<uint16_t> pressed_keys;

    for (auto key : _pressed_keys)
    {
        // F-клавиши не отправляем без модификаторов Shift, Ctrl или Alt
        if ((key >= KEY_F1) && (key <= KEY_F12) && (modifiers_size == 0))
        {
            continue;
        }

        pressed_keys.insert(key);
    }

    keysProcess(pressed_keys);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::processMouseInput()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::processControlPanelInput()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::processControl(const ControlType &ctrl_type)
{
    // В зависиомсти от типа обрабатываемого управления, вызываем тот или иной
    // метод обработки, видоизменяющий специфичные для данной ПЕ состяния органов управления.
    // Вызывается один какой-то метода, в зависиомсти от того, откуда пришел
    // управляющий сигнал
    switch (ctrl_type)
    {
    case CTRL_TYPE_KEYBOARD:

        processKeyBoardInput();

        break;

    case CTRL_TYPE_MOUSE:

        processMouseInput();

        break;

    case CTRL_TYPE_CTRL_PANEL:

        processControlPanelInput();

        break;
    }
}
