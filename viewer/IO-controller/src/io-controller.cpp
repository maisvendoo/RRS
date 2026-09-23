#include    <io-controller.h>
#include    <io-controller-keymap.h>
#include    <CfgReader.h>

#include    <QStringList>

#include    <algorithm>

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
    processControl(CTRL_TYPE_KEYBOARD);
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
void IOController::getHotkeysString(const QString &keyName, io_control_input_t &ic_input)
{
    if (!keyName.isEmpty())
    {
        ic_input.hot_keys = "Клавиши: ";

        if (!ic_input.keyModOnName.isEmpty())
        {
            ic_input.hot_keys += ic_input.keyModOnName + "+" + keyName.mid(4);
        }

        if (!ic_input.keyModOffName.isEmpty() && ic_input.keyModOnName != ic_input.keyModOffName)
        {
            ic_input.hot_keys += " | " + ic_input.keyModOffName + "+" + keyName.mid(4);
        }
    }
    else
    {
        ic_input.hot_keys = QString();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::getUsageString(io_control_input_t &ic_input)
{
    if (ic_input.type == "Toggle")
    {
        ic_input.usage = "Вкл.: ЛКМ | Выкл: ПКМ";
    }

    if (ic_input.type == "Button")
    {
        ic_input.usage = "Нажать: ЛКМ";
    }
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

        cfg.getString(secNode, "Name", ic_input.name);
        cfg.getString(secNode, "Type", ic_input.type);

        getUsageString(ic_input);

        cfg.getString(secNode, "Description", ic_input.description);

        int control_ID = 0;
        cfg.getInt(secNode, "ID", control_ID);
        ic_input.id = static_cast<uint16_t>(control_ID);

        double value1 = 0.0;
        cfg.getDouble(secNode, "value1", value1);
        double value2 = 0.0;
        cfg.getDouble(secNode, "value2", value2);

        // Старый формат (cabine*.xml): одиночное начальное значение
        double value = value1;
        cfg.getDouble(secNode, "value", value);

        QString keyName = "";
        cfg.getString(secNode, "KeyName", keyName);
        ic_input.keyCode = KeySymbolsRRSMap.value(keyName, KEY_Undefined);

        cfg.getString(secNode, "KeyModOnName", ic_input.keyModOnName);

        cfg.getString(secNode, "KeyModOffName", ic_input.keyModOffName);

        if (ic_input.keyModOffName.isEmpty())
        {
            ic_input.keyModOffName = ic_input.keyModOnName;
        }

        getHotkeysString(keyName, ic_input);

        // Метаданные органа: подсказка и семантика клика мышью
        int signal_id = -1;
        if (cfg.getInt(secNode, "SignalID", signal_id))
        {
            ic_input.signal_id = signal_id;
        }

        int signal_id2 = -1;
        if (cfg.getInt(secNode, "SignalID2", signal_id2))
        {
            ic_input.signal_id2 = signal_id2;
        }

        cfg.getString(secNode, "StateMode", ic_input.state_mode);
        cfg.getString(secNode, "StateNames", ic_input.state_names);

        // Один контроллер обслуживает одну кабину, общий xml читают
        // оба: в хэш попадает только своя кабина (имена мешей кабин
        // различаются, коллизий внутри хэша нет, но фильтр страхует
        // от дублей id). Общее имя без суффикса кабины забирают все.
        QString object_name = "";
        cfg.getString(secNode, "ObjectName", object_name);

        QString object_name_cab1 = "";
        cfg.getString(secNode, "ObjectNameCab1", object_name_cab1);

        QString object_name_cab2 = "";
        cfg.getString(secNode, "ObjectNameCab2", object_name_cab2);

        auto insert_for_cab = [&](const QString &mesh_name, int cab, float init_value)
        {
            if (mesh_name.isEmpty())
                return;

            if ((cabine_filter >= 0) && (cabine_filter != cab))
                return;

            ic_input.cabine_idx = cab;
            ic_input.contolledObjectName = mesh_name;
            ic_input.value = init_value;
            io_control_inputs.insert(ic_input.id, mesh_name, ic_input);
        };

        insert_for_cab(object_name, 0, static_cast<float>(value));
        insert_for_cab(object_name_cab1, 0, static_cast<float>(value1));
        insert_for_cab(object_name_cab2, 1, static_cast<float>(value2));

        secNode = cfg.getNextSection();
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::setCabineIndex(int vehicle_idx, int cab_idx)
{
    cabine_filter = cab_idx;

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
void IOController::setVehicleSignals(const std::vector<float> *vehicle_signals)
{
    this->vehicle_signals = vehicle_signals;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float IOController::getVehicleSignal(int signal_id) const
{
    if ((vehicle_signals == nullptr) || (signal_id < 0) ||
        (static_cast<size_t>(signal_id) >= vehicle_signals->size()))
    {
        return -1.0f;
    }

    return (*vehicle_signals)[static_cast<size_t>(signal_id)];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::optional<io_control_input_t> IOController::getInputByObject(const QString &object_name) const
{
    return io_control_inputs.getByKey2(object_name);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool IOController::findControl(const std::string &node_name, io_control_input_t &out) const
{
    if (node_name.empty())
    {
        return false;
    }

    const QString node = QString::fromStdString(node_name);

    for (const auto &[key1, key2, value] : io_control_inputs.getAll())
    {
        if (key2.isEmpty())
        {
            continue;
        }

        if ((node == key2) || node.endsWith(key2))
        {
            out = value;
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::mouseRelease(const QString &object_name)
{
    auto io_ctrl = io_control_inputs.getByKey2(object_name);

    if (!io_ctrl.has_value())
    {
        return;
    }

    if (io_ctrl.value().type == "Button")
    {
        io_ctrl.value().value = 0.0f;
        emitControl(io_ctrl.value());
    }
}

void IOController::mouseClick(const QString &object_name, int button)
{
    auto io_ctrl = io_control_inputs.getByKey2(object_name);

    if (!io_ctrl.has_value())
    {
        return;
    }

    processMouseControl(io_ctrl.value(), button);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::mouseProcessTumbler(io_control_input_t input,
                                       uint32_t button,
                                       bool is_pressed)
{
    auto io_ctrl = io_control_inputs.getByKey1(input.id);

    if (!io_ctrl) return;

    if (input.type == "Toggle")
    {
        if (button == IO_CTRL_LEFT_MOUSE_BUTTON && !input.toBool())
        {
            io_ctrl->value = 1.0f;
            io_control_inputs.updateByKey1(input.id, io_ctrl.value());
            emit sigSendVehicleControlCommand(io_ctrl->serialize());
        }

        if (button == IO_CTRL_RIGHT_MOUSE_BUTTON && input.toBool())
        {
            io_ctrl->value = 0.0f;
            io_control_inputs.updateByKey1(input.id, io_ctrl.value());
            emit sigSendVehicleControlCommand(io_ctrl->serialize());
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::mouseProcessButton(io_control_input_t input,
                                      uint32_t button,
                                      bool is_pressed)
{
    auto io_ctrl = io_control_inputs.getByKey1(input.id);

    if (!io_ctrl) return;

    if (is_pressed)
    {
        if (input.type == "Button")
        {
            if (button == IO_CTRL_LEFT_MOUSE_BUTTON)
            {
                io_ctrl->value = 1.0f;
                io_control_inputs.updateByKey1(input.id, io_ctrl.value());
                emit sigSendVehicleControlCommand(io_ctrl->serialize());
            }
        }
    }
    else
    {
        if (input.type == "Button")
        {
            if (button == IO_CTRL_LEFT_MOUSE_BUTTON)
            {
                io_ctrl->value = 0.0f;
                io_control_inputs.updateByKey1(input.id, io_ctrl.value());
                emit sigSendVehicleControlCommand(io_ctrl->serialize());
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::mouseInputProcess(io_control_input_t input, uint32_t button, bool is_pressed)
{
    // Обработка контрола типа "тумблер"
    mouseProcessTumbler(input, button, is_pressed);

    // Обработка контрола типа "кнопка"
    mouseProcessButton(input, button, is_pressed);

    // Вызываем кастомную обработку мышеввода
    processMouseInput(input, button, is_pressed);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::processSwitchBySignal(io_control_input_t &io_ctrl)
{
    float cur = getVehicleSignal(io_ctrl.signal_id);

    if (cur < 0.0f)
    {
        cur = io_ctrl.value;
    }

    io_ctrl.value = (cur < 0.5f) ? 1.0f : 0.0f;
    emitControl(io_ctrl);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::processMouseControl(io_control_input_t &io_ctrl, int button)
{
    (void) button;

    if ((io_ctrl.type == "Toggle") || (io_ctrl.type == "Button"))
    {
        processSwitchBySignal(io_ctrl);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::processMouseInput(io_control_input_t input, uint32_t button, bool is_pressed)
{
    (void) input;
    (void) button;
    (void) is_pressed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString IOController::getControlStateText(const io_control_input_t &io_ctrl,
                                          float state) const
{
    if ((io_ctrl.type == "Toggle") || (io_ctrl.type == "Button"))
    {
        return (state > 0.5f) ? QString(u8"состояние: включено")
                              : QString(u8"состояние: выключено");
    }

    if (!io_ctrl.state_names.isEmpty())
    {
        const QStringList names =
                io_ctrl.state_names.split(';', Qt::KeepEmptyParts);

        if (!names.isEmpty())
        {
            int idx = 0;

            if (io_ctrl.state_mode == "centered")
            {
                idx = (state < -0.5f) ? 0
                    : (state > 0.5f) ? static_cast<int>(names.size() - 1)
                    : static_cast<int>(names.size() / 2);
            }
            else if (io_ctrl.state_mode == "index")
            {
                idx = std::min(static_cast<int>(names.size() - 1),
                               static_cast<int>(state + 0.5f));
            }
            else
            {
                idx = std::min(static_cast<int>(names.size() - 1),
                               static_cast<int>(state * names.size()));
            }

            idx = std::clamp(idx, 0, static_cast<int>(names.size() - 1));

            return u8"положение: " + names[idx];
        }
    }

    return QString(u8"положение: %1%").arg(static_cast<int>(state * 100.0f));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString IOController::pickSyntheticControl(const std::string &mesh_name,
                                           float local_x,
                                           float local_y) const
{
    (void) mesh_name;
    (void) local_x;
    (void) local_y;

    return QString();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::emitControl(const io_control_input_t &io_ctrl)
{
    io_control_inputs.updateByKey1(io_ctrl.id, io_ctrl);
    emit sigSendVehicleControlCommand(io_ctrl.serialize());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool IOController::checkModKey(const QString &modKeyName, const std::set<uint16_t> &pressed_keys)
{
    if (modKeyName == "Shift")
    {
        return isShift(pressed_keys);
    }

    if (modKeyName == "Control")
    {
        return isControl(pressed_keys);
    }

    if (modKeyName == "Alt")
    {
        return isAlt(pressed_keys);
    }

    return false;
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
        // Модификаторы из конфига; без них - Shift включает, Ctrl выключает
        const bool mod_on = io_ctrl->keyModOnName.isEmpty()
                ? isShift(pressed_keys)
                : checkModKey(io_ctrl->keyModOnName, pressed_keys);

        if (mod_on)
        {
            io_ctrl->value = 1.0f;
            io_control_inputs.updateByKey1(control_id, io_ctrl.value());
            emit sigSendVehicleControlCommand(io_ctrl->serialize());
            return;
        }

        const bool mod_off = io_ctrl->keyModOffName.isEmpty()
                ? isControl(pressed_keys)
                : checkModKey(io_ctrl->keyModOffName, pressed_keys);

        if (mod_off)
        {
            io_ctrl->value = 0.0f;
            io_control_inputs.updateByKey1(control_id, io_ctrl.value());
            emit sigSendVehicleControlCommand(io_ctrl->serialize());
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::processButton(const uint16_t &control_id, const std::set<uint16_t> &pressed_keys)
{
    auto io_ctrl = io_control_inputs.getByKey1(control_id);

    if (getKeyState(pressed_keys, io_ctrl->keyCode))
    {
        if (checkModKey(io_ctrl->keyModOnName, pressed_keys) || io_ctrl->keyModOnName.isEmpty())
        {
            io_ctrl->value = 1.0f;
        }
    }
    else
    {
        io_ctrl->value = 0.0f;
    }

    io_control_inputs.updateByKey1(control_id, io_ctrl.value());
    emit sigSendVehicleControlCommand(io_ctrl->serialize());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::processKeyBoardInput()
{
    std::set<uint16_t> pressed_keys;

    if (!_pressed_keys.empty())
    {
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
            keysProcess(pressed_keys);
            return;
        }

        for (auto key : _pressed_keys)
        {
            // F-клавиши не отправляем без модификаторов Shift, Ctrl или Alt
            if ((key >= KEY_F1) && (key <= KEY_F12) && (modifiers_size == 0))
            {
                continue;
            }

            pressed_keys.insert(key);
        }
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
    // В зависимости от типа обрабатываемого управления, вызываем тот или иной
    // метод обработки, видоизменяющий специфичные для данной ПЕ состояния органов управления.
    // Вызывается один какой-то метода, в зависимости от того, откуда пришел
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
