#include    <io-controller.h>
#include    <io-controller-keymap.h>
#include    <CfgReader.h>
#include    <filesystem.h>

#include    <QStringList>
#include    <QDir>
#include    <QFile>
#include    <QFileInfo>
#include    <QDomDocument>

#include    <algorithm>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
IOController::IOController(QObject *parent) : QObject(parent)
{
    isModifier["Shift"] = [](const std::set<uint16_t> &pressed_keys) {
        return isShift(pressed_keys);
    };

    isModifier["Ctrl"] = [](const std::set<uint16_t> &pressed_keys) {
        return isControl(pressed_keys);
    };

    isModifier["Alt"] = [](const std::set<uint16_t> &pressed_keys) {
        return isAlt(pressed_keys);
    };
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
        int control_ID = 0;
        cfg.getInt(secNode, "ID", control_ID);
        uint16_t id = static_cast<uint16_t>(control_ID);

        auto stored = io_control_inputs.getByKey1(id);

        io_control_input_t ic_input;

        if (stored.has_value())
        {
            ic_input = stored.value();
        }

        cfg.getString(secNode, "Name", ic_input.name);
        cfg.getString(secNode, "Type", ic_input.type);

        getUsageString(ic_input);

        cfg.getString(secNode, "Description", ic_input.description);

        ic_input.id = id;

        double value1 = 0.0;
        cfg.getDouble(secNode, "value1", value1);
        double value2 = 0.0;
        cfg.getDouble(secNode, "value2", value2);

        double value = value1;
        cfg.getDouble(secNode, "value", value);

        QString keyName = "";
        cfg.getString(secNode, "KeyName", keyName);

        if (!keyName.isEmpty())
        {
            ic_input.keyCode = KeySymbolsRRSMap.value(keyName, KEY_Undefined);
        }

        QString mod_on = "";
        cfg.getString(secNode, "KeyModOnName", mod_on);

        if (!mod_on.isEmpty())
        {
            ic_input.keyModOnName = mod_on;
        }

        QString mod_off = "";
        cfg.getString(secNode, "KeyModOffName", mod_off);

        if (!mod_off.isEmpty())
        {
            ic_input.keyModOffName = mod_off;
        }

        if (ic_input.keyModOffName.isEmpty())
        {
            ic_input.keyModOffName = ic_input.keyModOnName;
        }

        getHotkeysString(keyName, ic_input);

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

        QString object_name = "";
        cfg.getString(secNode, "ObjectName", object_name);

        QString object_name_cab1 = "";
        cfg.getString(secNode, "ObjectNameCab1", object_name_cab1);

        QString object_name_cab2 = "";
        cfg.getString(secNode, "ObjectNameCab2", object_name_cab2);

        if (!object_name_cab1.isEmpty())
        {
            ic_input.contolledObjectName = object_name_cab1;
        }
        else if (!object_name.isEmpty() && ic_input.contolledObjectName.isEmpty())
        {
            ic_input.contolledObjectName = object_name;
        }

        if (!object_name_cab2.isEmpty())
        {
            ic_input.contolledObjectName2 = object_name_cab2;
        }
        else if (!object_name.isEmpty() && ic_input.contolledObjectName2.isEmpty()
                 && ic_input.contolledObjectName != object_name)
        {
            ic_input.contolledObjectName2 = object_name;
        }

        if (ic_input.value == 0.0f)
        {
            ic_input.value = static_cast<float>(value);
        }

        if (stored.has_value())
        {
            io_control_inputs.updateByKey1(id, ic_input);
        }
        else
        {
            QString key2 = ic_input.contolledObjectName;

            if (key2.isEmpty())
            {
                key2 = QString::number(id);
            }

            io_control_inputs.insert(id, key2, ic_input);
        }

        secNode = cfg.getNextSection();
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::create_animations_map(const QStringList &anim_dirs)
{
    FileSystem &fs = FileSystem::getInstance();
    auto data_dir = fs.getDataDir();
    QString anim_path = QString::fromStdString(data_dir) +
                        QDir::separator() + "animations";

    for (const auto &anim_dir : anim_dirs)
    {
        QString full_anim_path = anim_path + QDir::separator() + anim_dir;

        QDir dir(full_anim_path);
        QStringList files = dir.entryList(QStringList() << "*.xml", QDir::Files | QDir::NoDotAndDotDot);

        for (const auto &file_name : files)
        {
            if (file_name.isEmpty())
            {
                continue;
            }

            QFileInfo fileInfo(file_name);
            QString animation_name = fileInfo.baseName();

            QDomDocument doc;

            QFile file(full_anim_path + QDir::separator() + file_name);

            if (!file.open(QIODevice::ReadOnly))
            {
                continue;
            }

            doc.setContent(&file);

            file.close();

            auto signalIds = doc.elementsByTagName("SignalID");

            if (signalIds.size() > 0)
            {
                bool ok = false;
                int sid = signalIds.at(0).toElement().text().toInt(&ok);

                if (ok)
                {
                    animation_signals_map[animation_name] = static_cast<uint16_t>(sid);
                }
            }
        }
    }
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
float IOController::getSignalValueByName(const QString &objectName) const
{
    if (objectName.isEmpty() || feedback_signals == nullptr)
    {
        return 0.0f;
    }

    auto it = animation_signals_map.find(objectName);

    if (it != animation_signals_map.end())
    {
        uint16_t signal_id = it.value();

        if (signal_id < feedback_signals->size())
        {
            return (*feedback_signals)[signal_id];
        }
    }

    return 0.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float IOController::getSignalValueByID(uint16_t control_id, int cab_idx) const
{
    auto io_ctrl = io_control_inputs.getByKey1(control_id);

    if (!io_ctrl)
    {
        return 0.0f;
    }

    QString mesh_name = io_ctrl->contolledObjectName;

    if ((cab_idx == 1) && !io_ctrl->contolledObjectName2.isEmpty())
    {
        mesh_name = io_ctrl->contolledObjectName2;
    }

    return getSignalValueByName(mesh_name);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::optional<io_control_input_t> IOController::getInputByObject(const QString &object_name) const
{
    io_control_input_t out;

    if (findControl(object_name.toStdString(), out))
    {
        return out;
    }

    return std::nullopt;
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
        auto match_name = [&](const QString &mesh_name, int cab) -> bool
        {
            if (mesh_name.isEmpty())
            {
                return false;
            }

            if ((node == mesh_name) || node.endsWith(mesh_name))
            {
                out = value;
                out.cabine_idx = cab;
                out.contolledObjectName = mesh_name;
                return true;
            }

            return false;
        };

        if (match_name(value.contolledObjectName, 0))
        {
            return true;
        }

        if (match_name(value.contolledObjectName2, 1))
        {
            return true;
        }

        (void) key1;
        (void) key2;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::mouseRelease(const QString &object_name)
{
    io_control_input_t io_ctrl;
    findControl(object_name.toStdString(), io_ctrl);

    if (io_ctrl.type == "Button")
    {
        io_ctrl.value = 0.0f;
        emitControl(io_ctrl);
    }
}

void IOController::mouseClick(const QString &object_name, int button)
{
    io_control_input_t io_ctrl;

    if (!findControl(object_name.toStdString(), io_ctrl))
    {
        return;
    }

    processMouseControl(io_ctrl, button);
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
    mouseProcessTumbler(input, button, is_pressed);

    mouseProcessButton(input, button, is_pressed);

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
    return isModifier.value(modKeyName, [](const std::set<uint16_t> &) {return false;})(pressed_keys);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::processTumbler(const uint16_t &control_id,
                                  const std::set<uint16_t> &pressed_keys)
{
    auto io_ctrl = io_control_inputs.getByKey1(control_id);

    if (!io_ctrl)
    {
        return;
    }

    if (getKeyState(pressed_keys, io_ctrl->keyCode))
    {
        if (io_ctrl->keyModOnName == io_ctrl->keyModOffName)
        {
            if (checkModKey(io_ctrl->keyModOnName, pressed_keys))
            {
                io_ctrl->value = 1.0f - io_ctrl->value;
                io_control_inputs.updateByKey1(control_id, io_ctrl.value());
                emit sigSendVehicleControlCommand(io_ctrl->serialize());
                return;
            }
        }

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

    if (!io_ctrl)
    {
        return;
    }

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
