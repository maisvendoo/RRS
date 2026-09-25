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
void IOController::getHotkeysString(const QString &keyName, ControlHandler *ctrl_handler)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ControlHandler *IOController::create_handler(QString type, QDomNode secNode, CfgReader &cfg)
{
    ControlHandler *ctrl_handler = nullptr;

    if (type == "Toggle")
    {
        ctrl_handler = new ToggleHandler();
    }

    if (type == "Button")
    {
        ctrl_handler = new ButtonHandler();
    }

    ctrl_handler->load_config(cfg, secNode);

    return ctrl_handler;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool IOController::load_config(CfgReader &cfg)
{
    cfg.getInt("Common", "CabinesNum", cabs_num);

    for (int i = 0; i < cabs_num + 1; ++i)
    {
        DualKeyHash<uint16_t, QString, ControlHandler *> ctrl_handlers;
        control_handlers.push_back(ctrl_handlers);
    }

    auto secNode = cfg.getFirstSection("Control");

    while (!secNode.isNull())
    {
        QString type = "";
        cfg.getString(secNode, "Type", type);

        double value1 = 0.0;
        cfg.getDouble(secNode, "value1", value1);
        double value2 = 0.0;
        cfg.getDouble(secNode, "value2", value2);

        QString object_name_cab1 = "";
        cfg.getString(secNode, "ObjectNameCab1", object_name_cab1);

        QString object_name_cab2 = "";
        cfg.getString(secNode, "ObjectNameCab2", object_name_cab2);

        QString object_name = "";
        cfg.getString(secNode, "ObjectName", object_name);

        if (!object_name.isEmpty())
        {
            auto &ctrl_handlers = *(control_handlers.end() - 1);
            ControlHandler *ctrl_handler = create_handler(type, secNode, cfg);
            ctrl_handler->cabine_idx = control_handlers.size() - 1;
            ctrl_handler->contolledObjectName = object_name;
            ctrl_handler->value = value1;
            ctrl_handlers.insert(ctrl_handler->id, object_name, ctrl_handler);
        }

        if (!object_name_cab1.isEmpty() && cabs_num > 0)
        {
            ControlHandler *ctrl_handler = create_handler(type, secNode, cfg);
            ctrl_handler->cabine_idx = 0;
            ctrl_handler->contolledObjectName = object_name_cab1;
            ctrl_handler->value = value1;
            control_handlers[0].insert(ctrl_handler->id, object_name_cab1, ctrl_handler);
        }

        if (!object_name_cab2.isEmpty() && cabs_num > 1)
        {
            ControlHandler *ctrl_handler = create_handler(type, secNode, cfg);
            ctrl_handler->cabine_idx = 1;
            ctrl_handler->contolledObjectName = object_name_cab2;
            ctrl_handler->value = value2;
            control_handlers[1].insert(ctrl_handler->id, object_name_cab2, ctrl_handler);
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

    for (int cab_idx = 0; cab_idx < control_handlers.size(); ++cab_idx)
    {
        for (auto &[key1, key2, value] : control_handlers[cab_idx].getAll())
        {
            value->controlled_vehicle_idx = vehicle_idx;
            value->cabine_idx = cab_idx;

            control_handlers[cab_idx].updateByKey1(key1, value);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool IOController::findControl(const std::string &node_name, ControlHandler *&handler) const
{
    if (node_name.empty())
    {
        return false;
    }

    const QString node = QString::fromStdString(node_name);

    for (size_t i = 0; i < control_handlers.size(); ++i)
    {
        if (control_handlers[i].size() == 0)
        {
            continue;
        }

        for (const auto &[key1, key2, value] : control_handlers[i].getAll())
        {
            if (key2.isEmpty())
            {
                continue;
            }

            if ((node == key2) || node.endsWith(key2))
            {
                handler = value;
                return true;
            }
        }
    }

    return false;
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

    for (auto &ctrl_handlers : control_handlers)
    {
        for (const auto &[id, name, handler] : ctrl_handlers.getAll())
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
    for (auto &ctrl_handlers : control_handlers)
    {
        for (const auto &[id, name, handler] : ctrl_handlers.getAll())
        {
            handler->processKeyInput(pressed_keys);
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
