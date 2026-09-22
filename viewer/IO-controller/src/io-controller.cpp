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
            processKeyBoardInput();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::setReleasedKey(uint16_t keyBase)
{
    _pressed_keys.erase(keyBase);    
    processKeyBoardInput();
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

    cfg.getInt("Common", "CabinesNum", cabs_num);

    for (int i = 0; i < cabs_num + 1; ++i)
    {
        DualKeyHash<uint16_t, QString, io_control_input_t> io_ctrl_inputs;
        io_control_inputs.push_back(io_ctrl_inputs);
    }

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

        double value = 0.0;
        cfg.getDouble(secNode, "value", value);
        ic_input.value = static_cast<float>(value);

        QString keyName = "";
        cfg.getString(secNode, "KeyName", keyName);
        ic_input.keyCode = KeySymbolsRRSMap.value(keyName, KEY_Undefined);

        cfg.getString(secNode, "KeyModOnName", ic_input.keyModOnName);

        cfg.getString(secNode, "KeyModOffName", ic_input.keyModOffName);

        getHotkeysString(keyName, ic_input);        

        QString object_name_cab1 = "";
        cfg.getString(secNode, "ObjectNameCab1", object_name_cab1);

        QString object_name_cab2 = "";
        cfg.getString(secNode, "ObjectNameCab2", object_name_cab2);

        QString object_name = "";
        cfg.getString(secNode, "ObjectName", object_name);

        if (!object_name.isEmpty())
        {
            auto &io_ctrl_inputs = *(io_control_inputs.end() - 1);
            ic_input.cabine_idx = io_control_inputs.size() - 1;
            io_ctrl_inputs.insert(ic_input.id, object_name, ic_input);
        }

        if (!object_name_cab1.isEmpty() && cabs_num > 0)
        {
            ic_input.cabine_idx = 0;
            io_control_inputs[0].insert(ic_input.id, object_name_cab1, ic_input);
        }

        if (!object_name_cab2.isEmpty() && cabs_num > 1)
        {
            ic_input.cabine_idx = 1;
            io_control_inputs[1].insert(ic_input.id, object_name_cab2, ic_input);
        }

        secNode = cfg.getNextSection();
    }    

    // Debug: кол-во контролов
    if (!io_control_inputs.empty())
        fprintf(stderr, "DBG IO: cabs_num=%d vec_size=%zu hash0=%d hash1=%d\n",
                cabs_num, io_control_inputs.size(),
                io_control_inputs[0].size(), io_control_inputs[1].size());

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::setCabineIndex(int vehicle_idx, int cab_idx)
{
    /*for (auto &[key1, key2, value] : io_control_inputs.getAll())
    {
        value.controlled_vehicle_idx = vehicle_idx;
        value.cabine_idx = cab_idx;

        io_control_inputs.updateByKey1(key1, value);
    }*/
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

    for (size_t i = 0; i < io_control_inputs.size(); ++i)
    {
        if (io_control_inputs[i].size() == 0)
        {
            continue;
        }

        for (const auto &[key1, key2, value] : io_control_inputs[i].getAll())
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
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::mouseProcessTumbler(io_control_input_t input,
                                       uint32_t button,
                                       bool is_pressed)
{
    auto io_ctrl = io_control_inputs[input.cabine_idx].getByKey1(input.id);

    if (!io_ctrl) return;

    if (input.type == "Toggle")
    {
        if (button == IO_CTRL_LEFT_MOUSE_BUTTON && !input.toBool())
        {
            io_ctrl->value = 1.0f;
            io_control_inputs[input.cabine_idx].updateByKey1(input.id, io_ctrl.value());
            emit sigSendVehicleControlCommand(io_ctrl->serialize());
        }

        if (button == IO_CTRL_RIGHT_MOUSE_BUTTON && input.toBool())
        {
            io_ctrl->value = 0.0f;
            io_control_inputs[input.cabine_idx].updateByKey1(input.id, io_ctrl.value());
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
    auto io_ctrl = io_control_inputs[input.cabine_idx].getByKey1(input.id);

    if (!io_ctrl) return;

    if (is_pressed)
    {
        if (input.type == "Button")
        {
            if (button == IO_CTRL_LEFT_MOUSE_BUTTON)
            {
                io_ctrl->value = 1.0f;
                io_control_inputs[input.cabine_idx].updateByKey1(input.id, io_ctrl.value());
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
                io_control_inputs[input.cabine_idx].updateByKey1(input.id, io_ctrl.value());
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
void IOController::keysProcess(std::set<uint16_t> &pressed_keys)
{
    (void) pressed_keys;
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

    if (modKeyName == "Ctrl")
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
void IOController::processTumbler(size_t cab_idx,
                                  const uint16_t &control_id,
                                  const std::set<uint16_t> &pressed_keys)
{
    // Проверяем конкретный контрол
    auto io_ctrl = io_control_inputs[cab_idx].getByKey1(control_id);

    if (!io_ctrl) return;

    // Нажата ли его клавиша
    if (getKeyState(pressed_keys, io_ctrl->keyCode))
    {
        // Модификаторы включения и отключения одинаковы
        if (io_ctrl->keyModOnName == io_ctrl->keyModOffName)
        {
            if (checkModKey(io_ctrl->keyModOnName, pressed_keys))
            {
                // Просто инвертируем состояние тумблера
                io_ctrl->value = 1.0f - io_ctrl->value;
                io_control_inputs[cab_idx].updateByKey1(control_id, io_ctrl.value());
                emit sigSendVehicleControlCommand(io_ctrl->serialize());
                return;
            }
        }

        // Нажат модификатор включения?
        if (checkModKey(io_ctrl->keyModOnName, pressed_keys))
        {
            io_ctrl->value = 1.0f;
            io_control_inputs[cab_idx].updateByKey1(control_id, io_ctrl.value());
            emit sigSendVehicleControlCommand(io_ctrl->serialize());
            return;
        }

        // Нажат модификатор выключения?
        if (checkModKey(io_ctrl->keyModOffName, pressed_keys))
        {
            io_ctrl->value = 0.0f;
            io_control_inputs[cab_idx].updateByKey1(control_id, io_ctrl.value());
            emit sigSendVehicleControlCommand(io_ctrl->serialize());
            return;
        }        
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::processButton(size_t cab_idx,
                                 const uint16_t &control_id,
                                 const std::set<uint16_t> &pressed_keys)
{
    auto io_ctrl = io_control_inputs[cab_idx].getByKey1(control_id);

    if (!io_ctrl) return;

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

    io_control_inputs[cab_idx].updateByKey1(control_id, io_ctrl.value());
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
void IOController::processMouseInput(io_control_input_t input, uint32_t button, bool is_pressed)
{
    (void) input;
    (void) button;
    (void) is_pressed;

    /* Место для написания собственного бреда в модулях-наследниках */
}
