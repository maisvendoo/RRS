# Управление Switcher через IOController

## Контекст

`Switcher` и `SwitcherControl` — классы SDK (компонент `device`), реализующие
многопозиционные переключатели с поддержкой звука переключения и автовозврата
из крайних положений (spring return).

Текущая архитектура управления `SwitcherControl` использует легаси-модель
ввода: `setControl(std::set<uint16_t>* keys)` — указатель на набор нажатых
клавиш, приходящий от `UpdateControlToServerHandler`.

Новая архитектура: IOController отправляет **абсолютное положение**
переключателя (0.0–1.0) через единый TCP-протокол
(`STYPE_SEND_VEHICLE_CONTROL_COMMAND`). На стороне симулятора положение
пишется в `Vehicle::control_inputs[cab_idx][id]`, откуда его читает
`SwitcherControl` в `step()`.

## Принцип

- **Один сигнальный ID** на орган управления. IOController отправляет
  положение (float 0.0–1.0), а не приращение.
- **Аппаратные пульты** — совместимы: шлют положение 0.0–1.0 напрямую.
- **Feedback** — IOController читает текущее положение из `analogSignal`
  через `getSignalValueByName()` (см. `docs/feedback.md`).
- **Удержание клавиши/кнопки** — автоповтор с задержкой и интервалом.
- **Логика Switcher** вынесена в отдельный класс `SwitcherHandler`,
  IOController делегирует ему обработку.

## Формат конфига

Для Switcher не вводятся новые поля в `io_control_input_t`.
Переиспользуются существующие:

| Поле `io_control_input_t` | Назначение для Switcher |
|---|---|
| `keyCode` | Клавиша увеличения позиции (вперёд) |
| `keyModOnName` | Модификатор увеличения |
| `keyModOffName` | Модификатор уменьшения |
| `type` | `"Switcher"` |

`keyCodeDec` (клавиша уменьшения) хранится в `SwitcherHandler`,
не в структуре контрола.

```xml
<Control name="ControllerMode" type="Switcher" Description="Контроллер режимов">
    <ID>60</ID>
    <KeyNameInc>KEY_P</KeyNameInc>
    <KeyModInc>Shift</KeyModInc>
    <KeyNameDec>KEY_O</KeyNameDec>
    <KeyModDec>Shift</KeyModDec>
    <NumPositions>4</NumPositions>
    <ObjectNameCab1>Controller_Mode</ObjectNameCab1>
</Control>
```

| Параметр | Описание |
|---|---|
| `type="Switcher"` | Тип контрола — многопозиционный переключатель |
| `ID` | Идентификатор сигнала управления в `control_inputs` |
| `KeyNameInc` | Клавиша увеличения позиции (пишется в `keyCode`) |
| `KeyModInc` | Модификатор увеличения (пишется в `keyModOnName`) |
| `KeyNameDec` | Клавиша уменьшения позиции (хранится в `SwitcherHandler`) |
| `KeyModDec` | Модификатор уменьшения (пишется в `keyModOffName`) |
| `NumPositions` | Количество позиций переключателя |
| `ObjectNameCab1` | Имя 3D-объекта для мышиного ввода |

## Класс SwitcherHandler

Наследует `ControlHandler` (см. `docs/control-handler.md`). Владеет логикой
всех Switcher-контролов.

**Файл:** `viewer/IO-controller/include/switcher-handler.h`

```cpp
#ifndef SWITCHER_HANDLER_H
#define SWITCHER_HANDLER_H

#include "control-handler.h"

class CfgReader;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SwitcherHandler : public ControlHandler
{
    Q_OBJECT

public:

    explicit SwitcherHandler(QObject *parent = nullptr);

    bool loadConfig(CfgReader &cfg, int cabs_num) override;

    void processKeyInput(const std::set<uint16_t>& pressed_keys,
                         int cabine_idx, int vehicle_idx) override;

    void processMouseInput(const io_control_input_t& input,
                           uint32_t button, bool is_pressed) override;

    void step(float dt) override;

private:

    /// Данные одного Switcher-контрола
    struct SwitcherData
    {
        uint16_t id = 0;
        uint16_t keyCodeInc = 0;
        QString  keyModIncName = "";
        uint16_t keyCodeDec = 0;
        QString  keyModDecName = "";
        uint16_t numPositions = 2;
        QString  objectName = "";
        float    value = 0.0f;

        int      cabine_idx = 0;
        int      controlled_vehicle_idx = 0;
    };`

    /// Все Switcher-контролы, сгруппированные по кабинам
    std::vector<QMap<uint16_t, SwitcherData>> switchers;

    /// Поиск по имени 3D-объекта (для мыши)
    bool findByName(const QString& name, SwitcherData& out) const;

    /// Поиск по ID
    SwitcherData* findById(int cab_idx, uint16_t control_id);

    /// Отправить положение
    void sendPosition(SwitcherData& sw, int direction);

    /// Состояние удержания: control_id → (направление, время)
    QMap<uint16_t, std::pair<int, float>> hold_state;

    static constexpr float HOLD_DELAY        = 0.3f;
    static constexpr float REPEAT_INTERVAL   = 0.1f;
};

#endif // SWITCHER_HANDLER_H
```

**Файл:** `viewer/IO-controller/src/switcher-handler.cpp`

```cpp
#include "switcher-handler.h"
#include <CfgReader.h>
#include <io-controller-keymap.h>
#include <QDomNode>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SwitcherHandler::SwitcherHandler(QObject *parent) : ControlHandler(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SwitcherHandler::loadConfig(CfgReader &cfg, int cabs_num)
{
{
    switchers.resize(cabs_num + 1);

    auto secNode = cfg.getFirstSection("Control");

    while (!secNode.isNull())
    {
        QString type = "";
        cfg.getString(secNode, "Type", type);

        if (type == "Switcher")
        {
            SwitcherData sw;

            int id = 0;
            cfg.getInt(secNode, "ID", id);
            sw.id = static_cast<uint16_t>(id);

            int num_pos = 2;
            cfg.getInt(secNode, "NumPositions", num_pos);
            sw.numPositions = static_cast<uint16_t>(num_pos);

            // Клавиша увеличения
            QString keyNameInc = "";
            cfg.getString(secNode, "KeyNameInc", keyNameInc);
            sw.keyCodeInc = KeySymbolsRRSMap.value(keyNameInc, KEY_Undefined);
            cfg.getString(secNode, "KeyModInc", sw.keyModIncName);

            // Клавиша уменьшения
            QString keyNameDec = "";
            cfg.getString(secNode, "KeyNameDec", keyNameDec);
            sw.keyCodeDec = KeySymbolsRRSMap.value(keyNameDec, KEY_Undefined);
            cfg.getString(secNode, "KeyModDec", sw.keyModDecName);

            // Начальное положение
            double value1 = 0.0;
            cfg.getDouble(secNode, "value1", value1);
            sw.value = static_cast<float>(value1);

            // Объекты по кабинам
            QString objName = "";
            cfg.getString(secNode, "ObjectName", objName);
            QString objCab1 = "";
            cfg.getString(secNode, "ObjectNameCab1", objCab1);
            QString objCab2 = "";
            cfg.getString(secNode, "ObjectNameCab2", objCab2);

            if (!objName.isEmpty())
            {
                sw.cabine_idx = switchers.size() - 1;
                sw.objectName = objName;
                switchers[sw.cabine_idx].insert(sw.id, sw);
            }

            if (!objCab1.isEmpty() && cabs_num > 0)
            {
                sw.cabine_idx = 0;
                sw.objectName = objCab1;
                switchers[0].insert(sw.id, sw);
            }

            if (!objCab2.isEmpty() && cabs_num > 1)
            {
                sw.cabine_idx = 1;
                sw.objectName = objCab2;
                switchers[1].insert(sw.id, sw);
            }
        }

        secNode = cfg.getNextSection();
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherHandler::processKeyInput(const std::set<uint16_t>& pressed_keys,
                                      int cabine_idx,
                                      int vehicle_idx)
{
    if (cabine_idx < 0 || cabine_idx >= static_cast<int>(switchers.size()))
        return;

    for (auto& [id, sw] : switchers[cabine_idx])
    {
        sw.controlled_vehicle_idx = vehicle_idx;

        bool inc_down = getKeyState(pressed_keys, sw.keyCodeInc) &&
                        isKeyModifier(pressed_keys, sw.keyModIncName);
        bool dec_down = getKeyState(pressed_keys, sw.keyCodeDec) &&
                        isKeyModifier(pressed_keys, sw.keyModDecName);

        if (inc_down)
        {
            sendPosition(sw, +1);
            hold_state[sw.id] = {+1, 0.0f};
            return;
        }
        else if (dec_down)
        {
            sendPosition(sw, -1);
            hold_state[sw.id] = {-1, 0.0f};
            return;
        }
        else
        {
            hold_state[sw.id] = {0, 0.0f};
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherHandler::processMouseInput(const io_control_input_t& input,
                                        uint32_t button,
                                        bool is_pressed)
{
    SwitcherData sw;
    if (!findByName(input.contolledObjectName, sw))
        return;

    if (!is_pressed)
    {
        hold_state[sw.id] = {0, 0.0f};
        return;
    }

    int direction = 0;
    if (button == IO_CTRL_LEFT_MOUSE_BUTTON)
        direction = +1;
    else if (button == IO_CTRL_RIGHT_MOUSE_BUTTON)
        direction = -1;
    else
        return;

    sendPosition(sw, direction);
    hold_state[sw.id] = {direction, 0.0f};
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherHandler::step(float dt)
{
    for (auto& [id, state] : hold_state)
    {
        if (state.first == 0) continue;

        state.second += dt;

        if (state.second < HOLD_DELAY)
            continue;

        if (state.second < HOLD_DELAY + REPEAT_INTERVAL)
            continue;

        for (auto& cab_map : switchers)
        {
            auto it = cab_map.find(id);
            if (it != cab_map.end())
            {
                sendPosition(it.value(), state.first);
                state.second = HOLD_DELAY;
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherHandler::sendPosition(SwitcherData& sw, int direction)
{
    // Текущее положение из feedback
    float current = getSignalValueByName(sw.objectName);
    if (!feedback_signals)
        current = sw.value;

    float step = 1.0f / static_cast<float>(sw.numPositions - 1);
    int cur_idx = static_cast<int>(round(current / step));
    int new_idx = cur_idx + direction;

    if (new_idx < 0 || new_idx >= static_cast<int>(sw.numPositions))
        return;

    sw.value = static_cast<float>(new_idx) * step;

    // Отправка через базовый класс
    sendControlSignal(sw.controlled_vehicle_idx, sw.cabine_idx, sw.id, sw.value);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SwitcherHandler::findByName(const QString& name, SwitcherData& out) const
{
    if (name.isEmpty()) return false;

    for (const auto& cab_map : switchers)
    {
        for (const auto& [id, sw] : cab_map)
        {
            if (!sw.objectName.isEmpty() &&
                (name == sw.objectName || name.endsWith(sw.objectName)))
            {
                out = sw;
                return true;
            }
        }
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SwitcherHandler::SwitcherData* SwitcherHandler::findById(int cab_idx,
                                                          uint16_t control_id)
{
    if (cab_idx < 0 || cab_idx >= static_cast<int>(switchers.size()))
        return nullptr;

    auto it = switchers[cab_idx].find(control_id);
    if (it != switchers[cab_idx].end())
        return &it.value();

    return nullptr;
}
```

## Интеграция с IOController

IOController владеет списком `ControlHandler*`. `SwitcherHandler` создаётся
в `load_config()` и добавляется в общий список.

### В `load_config()`:

```cpp
// После загрузки всех контролов в DualKeyHash:

// Создаём и добавляем SwitcherHandler
SwitcherHandler* sw_handler = new SwitcherHandler(this);
sw_handler->loadConfig(cfg, cabs_num);
sw_handler->setAnimationSignalsMap(animation_signals_map);
sw_handler->setFeedbackSignals(feedback_signals);

connect(sw_handler, &ControlHandler::sigSendControlCommand,
        this, &IOController::sigSendVehicleControlCommand);

handlers.push_back(sw_handler);
```

### Делегирование вызовов:

```cpp
void IOController::processKeyBoardInput()
{
    // ... фильтрация pressed_keys ...

    for (auto* h : handlers)
        h->processKeyInput(pressed_keys, cabine_idx, vehicle_idx);
}

void IOController::mouseInputProcess(io_control_input_t input,
                                     uint32_t button, bool is_pressed)
{
    for (auto* h : handlers)
        h->processMouseInput(input, button, is_pressed);
}

void IOController::step(float t, float dt)
{
    for (auto* h : handlers)
        h->step(dt);
}

void IOController::setFeedbackSignals(const std::vector<float>* signals)
{
    feedback_signals = signals;
    for (auto* h : handlers)
        h->setFeedbackSignals(signals);
}
```

### Деструктор:

```cpp
IOController::~IOController()
{
    for (auto* h : handlers)
        delete h;
}
```

## Изменения в SwitcherControl (SDK)

### Новые поля и методы

**Файл:** `simulator/device/include/switcher-control.h`

```cpp
/// Задать указатель на массив управляющих сигналов от ТС
void setControlInputs(QMap<int, float>* inputs);

/// Задать ID сигнала управления положением
void setControlSignalID(uint16_t signal_id);
```

Protected:

```cpp
QMap<int, float>* control_inputs = nullptr;
uint16_t control_signal_id = KEY_Undefined;
```

### Модификация `step()`

**Файл:** `simulator/device/src/switcher-control.cpp`

```cpp
bool SwitcherControl::step(double t, double dt)
{
    bool changed = false;

    // ─── Новый режим: управление положением из control_inputs ───
    if (control_inputs && control_signal_id != KEY_Undefined)
    {
        float target = control_inputs->value(control_signal_id,
                                             getHandlePosition());
        uint16_t new_state = static_cast<uint16_t>(
            round(target * static_cast<float>(num_states - 1)));

        if (new_state != state)
        {
            state = new_state;
            switch_sound.play();
            changed = true;
        }
    }

    // ─── Легаси: управление с клавиатуры ───
    else if (pressed_keys && !pressed_keys->empty())
    {
        bool allow_spring_first = is_spring_first;
        bool allow_spring_last = is_spring_last;

        if (getKeyState(*pressed_keys, key_symbol_dec) &&
            isModifier(*pressed_keys, key_modifier_dec))
        {
            prev_key_inc = false;
            if (!prev_key_dec)
            {
                prev_key_dec = true;
                changed = decPos() || changed;
            }
            allow_spring_first = false;
        }
        else
        {
            prev_key_dec = false;
            if (getKeyState(*pressed_keys, key_symbol_inc) &&
                isModifier(*pressed_keys, key_modifier_inc))
            {
                if (!prev_key_inc)
                {
                    prev_key_inc = true;
                    changed = incPos() || changed;
                }
                allow_spring_last = false;
            }
            else
            {
                prev_key_inc = false;
            }
        }

        if (allow_spring_first && (state == 0))
            changed = incPos() || changed;
        if (allow_spring_last && (state == (num_states - 1)))
            changed = decPos() || changed;

        return changed;
    }

    // ─── Spring return (новый режим + режим без клавиш) ───
    if (is_spring_first && state == 0)
        changed = incPos() || changed;
    if (is_spring_last && state == num_states - 1)
        changed = decPos() || changed;

    return changed;
}
```

Реализация новых методов:

```cpp
void SwitcherControl::setControlInputs(QMap<int, float>* inputs)
{
    control_inputs = inputs;
}

void SwitcherControl::setControlSignalID(uint16_t signal_id)
{
    control_signal_id = signal_id;
}
```

## Сторона устройства (пример)

В устройстве, использующем `SwitcherControl`, в `loadConfig()`:

```cpp
void SomeDevice::loadConfig(QString cfg_path)
{
    // ...
    switcher.setControlInputs(&control_inputs);   // от Vehicle
    switcher.setControlSignalID(60);              // ID из IOControllerConfig
    // ...
}
```

В `step()` устройства вызов `switcher.step(t, dt)` уже обработает
и положение из `control_inputs`, и spring return.

## Поток данных (полный)

```
Viewer (IOController)                Simulator
─────────────────────                ────────

Нажатие Key_Inc (Shift+P):
  SwitcherHandler::processKeyInput()
    getSignalValueByName("Controller_Mode") → 0.33
    new_pos = round(0.33/0.33) + 1 → 2
    value = 2 * 0.33 → 0.66
    sigSendControlCommand(vehicle_idx, cab_idx, 60, 0.66)
       │
       ▼ TCP
                          Model::slotSetVehicleControlCommand()
                            vehicle->control_inputs[cab0][60] = 0.66
       │
       ▼
                          SomeDevice::step()
                            switcher.step(t, dt)
                              control_inputs[60] = 0.66
                              new_state = round(0.66 * 3) = 2
                              setPosition(2) → state = 2, sound.play()
       │
       ▼
                          prepareFeedBack()
                            analogSignal[signal_id_анимации] = 0.66
       │
       ▼ TCP
Viewer:
  VehicleExterior::step()
    io_controller->setFeedbackSignals(&analogSignal)
    ↓
  IOController пробрасывает всем handler-ам:
    for (auto* h : handlers)
      h->setFeedbackSignals(&analogSignal)
    ↓
  SwitcherHandler:
    getSignalValueByName() → 0.66
  ProcAnimation → analogSignal[signal_id]
```

## Параметры автоповтора

| Константа | Значение | Описание |
|---|---|---|
| `HOLD_DELAY` | 0.3 сек | Задержка перед началом автоповтора |
| `REPEAT_INTERVAL` | 0.1 сек | Интервал между шагами автоповтора |

## Обратная совместимость

- `SwitcherControl`: если `control_inputs == nullptr` — работает по-старому
  через `pressed_keys`
- IOController: все изменения в `SwitcherHandler`, существующие типы
  Toggle/Button не затронуты
- `io_control_input_t`: поля не добавляются, существующие не изменяются

## Влияние на SDK

| Компонент | Файлы | Изменения |
|---|---|---|
| `device` | `switcher-control.h`, `switcher-control.cpp` | Добавление новых методов (обратно совместимо) |
| `io-controller` | `control-handler.h` | Базовый класс `ControlHandler` |
| `io-controller` | `switcher-handler.h`, `switcher-handler.cpp` | Наследник `ControlHandler` |
| `io-controller` | `io-controller.h`, `io-controller.cpp` | Список `handlers` вместо прямой интеграции |

## План реализации (слайсы)

1. **ControlHandler — базовый класс:** `control-handler.h` с общей логикой
   (`getSignalValueByName`, `sendControlSignal`, `getKeyState`, `isKeyModifier`)
2. **ToggleHandler — рефакторинг:** вынести Toggle/Button из IOController
   в наследника `ControlHandler`
3. **IOController — список handler-ов:** заменить прямые вызовы на итерацию
   по `std::vector<ControlHandler*>`
4. **SwitcherHandler — база:** наследование от `ControlHandler`,
   загрузка конфига, `processKeyInput()`, `sendPosition()` (через базовый класс)
5. **SwitcherHandler — удержание:** `step()` с автоповтором
6. **SwitcherHandler — мышь:** `processMouseInput()`
7. **SwitcherControl (SDK):** добавить `control_inputs`, `control_signal_id`,
   модифицировать `step()`