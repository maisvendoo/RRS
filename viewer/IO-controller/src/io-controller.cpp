#include    <io-controller.h>
#include    <io-controller-keymap.h>
#include    <CfgReader.h>
#include    <filesystem.h>

#include    <toggle-handler.h>
#include    <button-handler.h>

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
    cfg.getInt("Common", "CabinesNum", cabs_num);

    for (int i = 0; i < cabs_num + 1; ++i)
    {
        DualKeyHash<uint16_t, QString, io_control_input_t> io_ctrl_inputs;
        io_control_inputs.push_back(io_ctrl_inputs);
    }

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
            ic_input.contolledObjectName = object_name;
            ic_input.value = value1;
            io_ctrl_inputs.insert(ic_input.id, object_name, ic_input);
        }

        if (!object_name_cab1.isEmpty() && cabs_num > 0)
        {
            ic_input.cabine_idx = 0;
            ic_input.contolledObjectName = object_name_cab1;
            ic_input.value = value1;
            io_control_inputs[0].insert(ic_input.id, object_name_cab1, ic_input);
        }

        if (!object_name_cab2.isEmpty() && cabs_num > 1)
        {
            ic_input.cabine_idx = 1;
            ic_input.contolledObjectName = object_name_cab2;
            ic_input.value = value2;
            io_control_inputs[1].insert(ic_input.id, object_name_cab2, ic_input);
        }

        secNode = cfg.getNextSection();
    }

    create_handlers();

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

    // Просматриваем все каталоги анимаций, заданные в конфиге ПЕ
    for (const auto &anim_dir : anim_dirs)
    {
        QString full_anim_path = anim_path + QDir::separator() + anim_dir;

        QDir dir(full_anim_path);
        QStringList files = dir.entryList(QStringList() << "*.xml",
                                          QDir::Files | QDir::NoDotAndDotDot);

        // Все xml-конкретного каталога
        for (const auto &file_name : files)
        {
            if (file_name.isEmpty())
            {
                continue;
            }

            QFileInfo fileInfo(file_name);

            // Ключ - имя анимации, оно же имя файла, оно же имя узла к которому
            // привязано мышиное управления
            QString animation_name = fileInfo.baseName();

            QDomDocument doc;

            QFile file(full_anim_path + QDir::separator() + file_name);

            if (!file.open(QIODevice::ReadOnly))
            {
                continue;
            }

            doc.setContent(&file);

            file.close();

            // Выдираем из файл SignalID
            auto signalIds = doc.elementsByTagName("SignalID");

            if (signalIds.size() > 0)
            {
                bool ok = false;
                int sid = signalIds.at(0).toElement().text().toInt(&ok);

                if (ok)
                {
                    // Сохраняем пару имя - идентификатор
                    animation_signals_map[animation_name] = static_cast<uint16_t>(sid);
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::setVehicleIndex(int vehicle_idx)
{
    this->vehicle_idx = vehicle_idx;

    for (int cab_idx = 0; cab_idx < io_control_inputs.size(); ++cab_idx)
    {
        for (auto &[key1, key2, value] : io_control_inputs[cab_idx].getAll())
        {
            value.controlled_vehicle_idx = vehicle_idx;
            value.cabine_idx = cab_idx;

            io_control_inputs[cab_idx].updateByKey1(key1, value);
        }
    }
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
void IOController::create_handlers()
{
    for (const auto& io_ctrl : io_control_inputs)
    {
        for (const auto & [id, name, input] : io_ctrl.getAll())
        {
            if (input.type == "Toggle")
            {
                ToggleHandler *toggle = new ToggleHandler();
                toggle->setControlInputs(&io_control_inputs);
                toggle->setAnimationSignalsMap(&animation_signals_map);

                connect(toggle, &ControlHandler::sigSendControlCommand,
                        this, &IOController::sigSendVehicleControlCommand);

                handlers.insert(id, toggle);
            }

            if (input.type == "Button")
            {
                ButtonHandler *button = new ButtonHandler();
                button->setControlInputs(&io_control_inputs);
                button->setAnimationSignalsMap(&animation_signals_map);

                connect(button, &ControlHandler::sigSendControlCommand,
                        this, &IOController::sigSendVehicleControlCommand);

                handlers.insert(id, button);
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::mouseInputProcess(io_control_input_t input, uint32_t button, bool is_pressed)
{
    for (auto *handler : handlers)
    {
        if (handler != nullptr)
        {
            handler->processMouseInput(input, button, is_pressed);
        }
    }

    processMouseInput(input, button, is_pressed);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::setFeedbackSignals(const std::vector<float> *server_signals)
{
    if (server_signals == nullptr)
    {
        return;
    }

    for (auto *handler : handlers)
    {
        if (handler != nullptr)
        {
            handler->setFeedbackSignals(server_signals);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::processKeyboardInput(std::set<uint16_t> &pressed_keys)
{
    (void) pressed_keys;

    /* Место для написания собственного бреда в модулях-наследниках */
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::keyboardInputProcess(std::set<uint16_t> &pressed_keys)
{
    for (auto *handler : handlers)
    {
        if (handler != nullptr)
        {
            handler->processKeyInput(pressed_keys, cabine_idx, vehicle_idx);
        }
    }

    processKeyboardInput(pressed_keys);
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
            keyboardInputProcess(pressed_keys);
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

    keyboardInputProcess(pressed_keys);
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
