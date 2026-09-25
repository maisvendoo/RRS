# ControlHandler — базовая архитектура обработчиков управления

## Мотивация

IOController исторически монолитен: вся логика Toggle, Button и вновь
добавляемого Switcher живёт в одном классе. Это затрудняет расширение.

Решение: выделить базовый класс `ControlHandler` и для каждого типа контрола
создать отдельного наследника. Handler-ы инкапсулируют **только логику**
обработки ввода (клавиатура, мышь). **Состояние** контролов хранится
в `io_control_inputs` (DualKeyHash), принадлежащем IOController.

## Принцип разделения

```
IOController (владелец данных)
  ├── io_control_inputs (DualKeyHash) — состояние всех контролов
  │     [cab0][id=100] → io_control_input_t { keyCode, mods, value, ... }
  │     [cab0][id=60]  → io_control_input_t { type, ... }
  │     [shared][...]
  │
  └── handlers (список ControlHandler*) — только логика
        ├── ToggleHandler  — processTumbler / processButton
        └── SwitcherHandler — inc/dec, автоповтор
```

Handler-ы **не хранят копии данных**. Они получают указатель на
DualKeyHash и читают/пишут состояние напрямую.

### Исключение

`SwitcherHandler` хранит **только расширенные поля**, которых нет
в `io_control_input_t` (keyCodeDec, numPositions). Основные поля
(id, keyCodeInc, objectName) читает из DualKeyHash.

## Базовый класс ControlHandler

**Файл:** `viewer/IO-controller/include/control-handler.h`

```cpp
#ifndef CONTROL_HANDLER_H
#define CONTROL_HANDLER_H

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QString>
#include <cstdint>
#include <set>
#include <vector>

#include <io-controller-input.h>
#include <dual-key-hash.h>

class CfgReader;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ControlHandler : public QObject
{
    Q_OBJECT

public:

    explicit ControlHandler(QObject *parent = nullptr) : QObject(parent) {}

    virtual ~ControlHandler() = default;

    /// Установить указатель на DualKeyHash с данными контролов
    void setControlInputs(
        std::vector<DualKeyHash<uint16_t, QString, io_control_input_t>>* inputs)
    {
        ctrl_inputs = inputs;
    }

    /// Загрузка конфигурации из IOControllerConfig
    virtual bool loadConfig(CfgReader &cfg, int cabs_num) = 0;

    /// Обработка клавиатурного ввода
    virtual void processKeyInput(const std::set<uint16_t>& pressed_keys,
                                 int cabine_idx, int vehicle_idx) = 0;

    /// Обработка мышиного ввода
    virtual void processMouseInput(const io_control_input_t& input,
                                   uint32_t button, bool is_pressed) = 0;

    /// Шаг симуляции (для автоповтора и т.п.)
    virtual void step(float dt) {}

    /// Установить массив сигналов обратной связи
    virtual void setFeedbackSignals(const std::vector<float>* signals)
    {
        feedback_signals = signals;
    }

    /// Установить карту анимационных сигналов
    virtual void setAnimationSignalsMap(const QMap<QString, uint16_t>& map)
    {
        animation_signals_map = map;
    }

signals:

    /// Сигнал отправки команды управления
    void sigSendControlCommand(const QByteArray &data);

protected:

    /// Указатель на DualKeyHash IOController (данные и состояние)
    std::vector<DualKeyHash<uint16_t, QString, io_control_input_t>>* ctrl_inputs = nullptr;

    /// Указатель на массив аналоговых сигналов от симулятора
    const std::vector<float>* feedback_signals = nullptr;

    /// Маппинг: имя 3D-объекта → ID сигнала обратной связи
    QMap<QString, uint16_t> animation_signals_map;

    /// Получить значение сигнала по имени объекта из feedback_signals
    float getSignalValueByName(const QString& objectName) const
    {
        if (objectName.isEmpty() || !feedback_signals)
            return 0.0f;

        auto it = animation_signals_map.find(objectName);
        if (it != animation_signals_map.end())
        {
            if (it.value() < feedback_signals->size())
                return (*feedback_signals)[it.value()];
        }

        for (auto it = animation_signals_map.begin();
             it != animation_signals_map.end(); ++it)
        {
            if (objectName.endsWith(it.key()))
            {
                if (it.value() < feedback_signals->size())
                    return (*feedback_signals)[it.value()];
            }
        }

        return 0.0f;
    }

    /// Проверка состояния клавиши
    static bool getKeyState(const std::set<uint16_t>& keys, uint16_t key)
    {
        if (key == 0 || key == KEY_Undefined) return false;
        return keys.find(key) != keys.end();
    }

    /// Проверка модификатора
    static bool isKeyModifier(const std::set<uint16_t>& keys,
                              const QString& modName)
    {
        if (modName.isEmpty()) return true;
        if (modName == "Shift") return isShift(keys);
        if (modName == "Ctrl")  return isControl(keys);
        if (modName == "Alt")   return isAlt(keys);
        return false;
    }

    /// Сериализация и отправка
    void sendControlSignal(int vehicle_idx, int cab_idx,
                           uint16_t id, float value)
    {
        io_control_input_t out;
        out.controlled_vehicle_idx = vehicle_idx;
        out.cabine_idx = cab_idx;
        out.id = id;
        out.value = value;

        emit sigSendControlCommand(out.serialize());
    }
};

#endif // CONTROL_HANDLER_H
```

## Пример: ToggleHandler

Логика обработки Toggle и Button. Не хранит собственных данных —
читает/пишет состояние в `ctrl_inputs` (DualKeyHash).

**Файл:** `viewer/IO-controller/include/toggle-handler.h`

```cpp
#ifndef TOGGLE_HANDLER_H
#define TOGGLE_HANDLER_H

#include "control-handler.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ToggleHandler : public ControlHandler
{
    Q_OBJECT

public:

    explicit ToggleHandler(QObject *parent = nullptr);

    /// loadConfig — ничего не делает, все данные уже загружены в DualKeyHash
    bool loadConfig(CfgReader &cfg, int cabs_num) override;

    void processKeyInput(const std::set<uint16_t>& pressed_keys,
                         int cabine_idx, int vehicle_idx) override;

    void processMouseInput(const io_control_input_t& input,
                           uint32_t button, bool is_pressed) override;

    /// Публичные методы для прямого вызова из keysProcess() наследников
    /// (например, VL60IOController задаёт порядок контролов)
    void processToggle(size_t cab_idx, uint16_t control_id,
                       const std::set<uint16_t>& pressed_keys);

    void processButton(size_t cab_idx, uint16_t control_id,
                       const std::set<uint16_t>& pressed_keys);
};

#endif // TOGGLE_HANDLER_H
```

**Файл:** `viewer/IO-controller/src/toggle-handler.cpp`

```cpp
#include "toggle-handler.h"
#include <CfgReader.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ToggleHandler::ToggleHandler(QObject *parent) : ControlHandler(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ToggleHandler::loadConfig(CfgReader &cfg, int cabs_num)
{
    return true; // данные уже загружены IOController в DualKeyHash
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ToggleHandler::processKeyInput(const std::set<uint16_t>& pressed_keys,
                                    int cabine_idx, int vehicle_idx)
{
    if (!ctrl_inputs) return;

    for (const auto& [id, _, input] : (*ctrl_inputs)[cabine_idx].getAll())
    {
        if (input.type == "Toggle")
            processToggle(cabine_idx, id, pressed_keys);
        else if (input.type == "Button")
            processButton(cabine_idx, id, pressed_keys);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ToggleHandler::processToggle(size_t cab_idx, uint16_t control_id,
                                  const std::set<uint16_t>& pressed_keys)
{
    if (!ctrl_inputs) return;

    auto io_ctrl = (*ctrl_inputs)[cab_idx].getByKey1(control_id);
    if (!io_ctrl || io_ctrl->type != "Toggle") return;

    if (getKeyState(pressed_keys, io_ctrl->keyCode))
    {
        if (io_ctrl->keyModOnName == io_ctrl->keyModOffName)
        {
            if (isKeyModifier(pressed_keys, io_ctrl->keyModOnName))
            {
                io_ctrl->value = 1.0f - io_ctrl->value;
                (*ctrl_inputs)[cab_idx].updateByKey1(control_id, io_ctrl.value());
                sendControlSignal(io_ctrl->controlled_vehicle_idx, cab_idx,
                                  control_id, io_ctrl->value);
                return;
            }
        }

        if (isKeyModifier(pressed_keys, io_ctrl->keyModOnName))
        {
            io_ctrl->value = 1.0f;
            (*ctrl_inputs)[cab_idx].updateByKey1(control_id, io_ctrl.value());
            sendControlSignal(io_ctrl->controlled_vehicle_idx, cab_idx,
                              control_id, io_ctrl->value);
            return;
        }

        if (isKeyModifier(pressed_keys, io_ctrl->keyModOffName))
        {
            io_ctrl->value = 0.0f;
            (*ctrl_inputs)[cab_idx].updateByKey1(control_id, io_ctrl.value());
            sendControlSignal(io_ctrl->controlled_vehicle_idx, cab_idx,
                              control_id, io_ctrl->value);
            return;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ToggleHandler::processButton(size_t cab_idx, uint16_t control_id,
                                  const std::set<uint16_t>& pressed_keys)
{
    if (!ctrl_inputs) return;

    auto io_ctrl = (*ctrl_inputs)[cab_idx].getByKey1(control_id);
    if (!io_ctrl || io_ctrl->type != "Button") return;

    if (getKeyState(pressed_keys, io_ctrl->keyCode))
    {
        if (isKeyModifier(pressed_keys, io_ctrl->keyModOnName) ||
            io_ctrl->keyModOnName.isEmpty())
        {
            io_ctrl->value = 1.0f;
        }
    }
    else
    {
        io_ctrl->value = 0.0f;
    }

    (*ctrl_inputs)[cab_idx].updateByKey1(control_id, io_ctrl.value());
    sendControlSignal(io_ctrl->controlled_vehicle_idx, cab_idx,
                      control_id, io_ctrl->value);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ToggleHandler::processMouseInput(const io_control_input_t& input,
                                      uint32_t button, bool is_pressed)
{
    if (!ctrl_inputs) return;

    auto io_ctrl = (*ctrl_inputs)[input.cabine_idx].getByKey1(input.id);
    if (!io_ctrl) return;

    if (io_ctrl->type == "Toggle")
    {
        if (button == IO_CTRL_LEFT_MOUSE_BUTTON && !input.toBool())
        {
            io_ctrl->value = 1.0f;
            (*ctrl_inputs)[input.cabine_idx].updateByKey1(input.id, io_ctrl.value());
            sendControlSignal(io_ctrl->controlled_vehicle_idx,
                              input.cabine_idx, input.id, io_ctrl->value);
        }

        if (button == IO_CTRL_RIGHT_MOUSE_BUTTON && input.toBool())
        {
            io_ctrl->value = 0.0f;
            (*ctrl_inputs)[input.cabine_idx].updateByKey1(input.id, io_ctrl.value());
            sendControlSignal(io_ctrl->controlled_vehicle_idx,
                              input.cabine_idx, input.id, io_ctrl->value);
        }
    }
    else if (io_ctrl->type == "Button")
    {
        if (is_pressed && button == IO_CTRL_LEFT_MOUSE_BUTTON)
        {
            io_ctrl->value = 1.0f;
            (*ctrl_inputs)[input.cabine_idx].updateByKey1(input.id, io_ctrl.value());
            sendControlSignal(io_ctrl->controlled_vehicle_idx,
                              input.cabine_idx, input.id, io_ctrl->value);
        }
        else if (!is_pressed && button == IO_CTRL_LEFT_MOUSE_BUTTON)
        {
            io_ctrl->value = 0.0f;
            (*ctrl_inputs)[input.cabine_idx].updateByKey1(input.id, io_ctrl.value());
            sendControlSignal(io_ctrl->controlled_vehicle_idx,
                              input.cabine_idx, input.id, io_ctrl->value);
        }
    }
}
```

## IOController с ControlHandler

**Файл:** `viewer/IO-controller/include/io-controller.h`

```cpp
class IOController : public QObject
{
    Q_OBJECT

public:

    IOController(QObject *parent = nullptr);
    ~IOController();

    void setPressedKey(uint16_t keyBase);
    void setReleasedKey(uint16_t keyBase);
    virtual void step(float t, float dt);
    virtual bool load_config(CfgReader &cfg);
    void create_animations_map(const QStringList &anim_dirs);
    void setVehicleIndex(int vehicle_idx);
    void setActirveCabineIndex(int cab_idx) { cabine_idx = cab_idx; }
    bool findControl(const std::string &node_name, io_control_input_t &out) const;
    void mouseInputProcess(io_control_input_t input, uint32_t button, bool is_pressed);
    void setFeedbackSignals(const std::vector<float> *server_signals);

signals:
    void sigSendVehicleControlCommand(const QByteArray &data);

protected:
    std::set<uint16_t> _pressed_keys;
    int cabine_idx = 0;
    int vehicle_idx = 0;

    QMap<QString, std::function<bool(const std::set<uint16_t> &)>> isModifier;

    /// Единое хранилище данных и состояния всех контролов
    std::vector<DualKeyHash<uint16_t, QString, io_control_input_t>> io_control_inputs;
    int cabs_num = 0;

    virtual void keysProcess(std::set<uint16_t> &pressed_keys);
    virtual void processMouseInput(io_control_input_t input, uint32_t button, bool is_pressed);

private:
    std::vector<ControlHandler*> handlers;

    QMap<QString, uint16_t> animation_signals_map;
    const std::vector<float>* feedback_signals = nullptr;

    void load_handlers(CfgReader &cfg);
    void processKeyBoardInput();
};

extern "C" IOController* createIOController()
{
    return new IOController();
}
```

**Файл:** `viewer/IO-controller/src/io-controller.cpp`

```cpp
IOController::IOController(QObject *parent) : QObject(parent)
{
    isModifier["Shift"] = [](const std::set<uint16_t> &k) { return isShift(k); };
    isModifier["Ctrl"]  = [](const std::set<uint16_t> &k) { return isControl(k); };
    isModifier["Alt"]   = [](const std::set<uint16_t> &k) { return isAlt(k); };
}

IOController::~IOController()
{
    for (auto* h : handlers)
        delete h;
}

bool IOController::load_config(CfgReader &cfg)
{
    cfg.getInt("Common", "CabinesNum", cabs_num);

    for (int i = 0; i < cabs_num + 1; ++i)
    {
        DualKeyHash<uint16_t, QString, io_control_input_t> io_ctrl_inputs;
        io_control_inputs.push_back(io_ctrl_inputs);
    }

    // Загружаем ВСЕ контролы в DualKeyHash
    auto secNode = cfg.getFirstSection("Control");
    while (!secNode.isNull())
    {
        io_control_input_t ic_input;

        cfg.getString(secNode, "Type", ic_input.type);
        cfg.getString(secNode, "Name", ic_input.name);
        cfg.getString(secNode, "Description", ic_input.description);

        int id = 0;
        cfg.getInt(secNode, "ID", id);
        ic_input.id = static_cast<uint16_t>(id);

        double value1 = 0.0, value2 = 0.0;
        cfg.getDouble(secNode, "value1", value1);
        cfg.getDouble(secNode, "value2", value2);

        QString keyName = "";
        cfg.getString(secNode, "KeyName", keyName);
        ic_input.keyCode = KeySymbolsRRSMap.value(keyName, KEY_Undefined);

        cfg.getString(secNode, "KeyModOnName", ic_input.keyModOnName);
        cfg.getString(secNode, "KeyModOffName", ic_input.keyModOffName);
        if (ic_input.keyModOffName.isEmpty())
            ic_input.keyModOffName = ic_input.keyModOnName;

        QString objName = "";
        cfg.getString(secNode, "ObjectName", objName);
        QString objCab1 = "";
        cfg.getString(secNode, "ObjectNameCab1", objCab1);
        QString objCab2 = "";
        cfg.getString(secNode, "ObjectNameCab2", objCab2);

        if (!objName.isEmpty())
        {
            ic_input.cabine_idx = io_control_inputs.size() - 1;
            ic_input.contolledObjectName = objName;
            ic_input.value = value1;
            io_control_inputs.back().insert(ic_input.id, objName, ic_input);
        }

        if (!objCab1.isEmpty() && cabs_num > 0)
        {
            ic_input.cabine_idx = 0;
            ic_input.contolledObjectName = objCab1;
            ic_input.value = value1;
            io_control_inputs[0].insert(ic_input.id, objCab1, ic_input);
        }

        if (!objCab2.isEmpty() && cabs_num > 1)
        {
            ic_input.cabine_idx = 1;
            ic_input.contolledObjectName = objCab2;
            ic_input.value = value2;
            io_control_inputs[1].insert(ic_input.id, objCab2, ic_input);
        }

        secNode = cfg.getNextSection();
    }

    // Создаём handler-ы
    load_handlers(cfg);

    return true;
}

void IOController::load_handlers(CfgReader &cfg)
{
    // Определяем, какие handler-ы нужны, по типам в конфиге
    bool need_toggle = false;
    bool need_switcher = false;

    auto secNode = cfg.getFirstSection("Control");
    while (!secNode.isNull())
    {
        QString type = "";
        cfg.getString(secNode, "Type", type);
        if (type == "Toggle" || type == "Button") need_toggle = true;
        if (type == "Switcher") need_switcher = true;
        secNode = cfg.getNextSection();
    }

    // Создаём handler-ы и даём им доступ к DualKeyHash
    if (need_toggle)
    {
        ToggleHandler* toggle = new ToggleHandler(this);
        toggle->setControlInputs(&io_control_inputs);
        toggle->setAnimationSignalsMap(animation_signals_map);
        toggle->setFeedbackSignals(feedback_signals);
        connect(toggle, &ControlHandler::sigSendControlCommand,
                this, &IOController::sigSendVehicleControlCommand);
        handlers.push_back(toggle);
    }

    if (need_switcher)
    {
        SwitcherHandler* sw = new SwitcherHandler(this);
        sw->loadConfig(cfg, cabs_num); // загружает расширенные поля
        sw->setControlInputs(&io_control_inputs);
        sw->setAnimationSignalsMap(animation_signals_map);
        sw->setFeedbackSignals(feedback_signals);
        connect(sw, &ControlHandler::sigSendControlCommand,
                this, &IOController::sigSendVehicleControlCommand);
        handlers.push_back(sw);
    }
}
```

Делегирование вызовов handler-ам:

```cpp
void IOController::processKeyBoardInput()
{
    // фильтрация pressed_keys (как сейчас)
    std::set<uint16_t> pressed_keys;
    /* ... */

    for (auto* handler : handlers)
        handler->processKeyInput(pressed_keys, cabine_idx, vehicle_idx);

    keysProcess(pressed_keys);
}

void IOController::mouseInputProcess(io_control_input_t input,
                                     uint32_t button, bool is_pressed)
{
    for (auto* handler : handlers)
        handler->processMouseInput(input, button, is_pressed);

    processMouseInput(input, button, is_pressed);
}

void IOController::step(float t, float dt)
{
    for (auto* handler : handlers)
        handler->step(dt);
}

void IOController::setFeedbackSignals(const std::vector<float>* signals)
{
    feedback_signals = signals;
    for (auto* handler : handlers)
        handler->setFeedbackSignals(signals);
}
```

## Миграция VL60IOController (Вариант 2)

`VL60IOController` — наследник `IOController`, переопределяющий `keysProcess()`.
При переходе на handler-ы меняет `processTumbler()` на `toggle_handler->processToggle()`:

```cpp
class VL60IOController : public IOController
{
    ToggleHandler* toggle_handler = nullptr;

public:
    VL60IOController()
    {
        // ToggleHandler будет создан в load_config(), получаем ссылку
    }

    void postInit()
    {
        // Находим ToggleHandler в списке handlers
        for (auto* h : handlers)
        {
            toggle_handler = dynamic_cast<ToggleHandler*>(h);
            if (toggle_handler) break;
        }
    }

    void keysProcess(std::set<uint16_t> &pressed_keys) override
    {
        toggle_handler->processToggle(cabine_idx, CTRL_TUMBLER_PNT, pressed_keys);
        toggle_handler->processToggle(cabine_idx, CTRL_TUMBLER_PNT1, pressed_keys);
        toggle_handler->processToggle(cabine_idx, CTRL_TUMBLER_PNT2, pressed_keys);
        toggle_handler->processToggle(cabine_idx, CTRL_MAIN_SWITCH_ON, pressed_keys);
        toggle_handler->processButton(cabine_idx, CTRL_RETURN_PROTECTION, pressed_keys);
        // ... все ID как сейчас
    }
};
```

Поведение идентично текущему:
- Данные (keyCode, модификаторы, value) — в DualKeyHash, как и сейчас
- `processToggle()` — та же логика, что и `processTumbler()`
- `processButton()` — та же логика, что и `processButton()` в IOController
- Сигнал отправляется handler-ом, IOController его пробрасывает дальше

## Преимущества

| Аспект | Было | Стало |
|---|---|---|
| Данные | `io_control_inputs` + дублирование | Только `io_control_inputs` (DualKeyHash) |
| Логика Toggle | в IOController | ToggleHandler |
| Логика Switcher | в IOController | SwitcherHandler |
| Расширение | править IOController | новый наследник ControlHandler |
| Кастомный порядок | `keysProcess()` override | `toggle_handler->processToggle(id, ...)` |

## Миграция (этапы)

1. **ControlHandler** — базовый класс с `setControlInputs()`, `getSignalValueByName()`,
   `sendControlSignal()`, `getKeyState()`, `isKeyModifier()`
2. **ToggleHandler** — логика Toggle/Button, работает через `ctrl_inputs`
3. **IOController** — `load_config()` заполняет DualKeyHash,
   `load_handlers()` создаёт handler-ы, передаёт им указатель на DualKeyHash
4. **IOController** — делегирует `processKeyInput`, `processMouseInput`, `step` handler-ам
5. **VL60IOController** — `keysProcess()` вызывает `toggle_handler->processToggle()`
   вместо `processTumbler()`