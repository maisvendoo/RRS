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
- **Логика Switcher** — в `SwitcherHandler` (наследник `ControlHandler`).
- **Состояние** всех контролов хранится в `io_control_inputs` (DualKeyHash),
  принадлежащем IOController. Handler не дублирует данные, а читает/пишет
  в DualKeyHash через указатель `ctrl_inputs`.

## Формат конфига

Для Switcher не вводятся новые поля в `io_control_input_t`.
Переиспользуются существующие:

| Поле `io_control_input_t` | Назначение для Switcher |
|---|---|
| `keyCode` | Клавиша увеличения позиции (вперёд) |
| `keyModOnName` | Модификатор увеличения |
| `keyModOffName` | Модификатор уменьшения |
| `type` | `"Switcher"` |

`keyCodeDec` (клавиша уменьшения) и `numPositions` хранятся в
`SwitcherHandler` — это расширенные поля, которых нет в
`io_control_input_t`.

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

Наследует `ControlHandler` (см. `docs/control-handler.md`).

**Данные:**
- Основные поля (id, keyCodeInc, keyModIncName, keyModOffName, objectName, value)
  читаются из `ctrl_inputs` (DualKeyHash).
- Расширенные поля (`keyCodeDec`, `numPositions`) хранятся в своей карте.

**Логика:** обработка inc/dec, автоповтор при удержании.

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

    /// Расширенные данные Switcher (чего нет в io_control_input_t)
    struct SwitcherExt
    {
        uint16_t keyCodeDec = 0;
        QString  keyModDecName = "";
        uint16_t numPositions = 2;
    };

    /// Расширение: control_id → SwitcherExt (ключ — id из DualKeyHash)
    QMap<uint16_t, SwitcherExt> switcher_ext;

    /// Состояние удержания: control_id → (направление, время)
    QMap<uint16_t, std::pair<int, float>> hold_state;

    static constexpr float HOLD_DELAY        = 0.3f;
    static constexpr float REPEAT_INTERVAL   = 0.1f;

    /// Отправить следующую позицию
    void sendNextPosition(const QString& objectName, uint16_t control_id,
                          int cab_idx, int vehicle_idx, int direction,
                          uint16_t num_positions);
};

#endif // SWITCHER_HANDLER_H
```

**Файл:** `viewer/IO-controller/src/switcher-handler.cpp`

```cpp
#include "switcher-handler.h"
#include <CfgReader.h>
#include <io-controller-keymap.h>

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
    auto secNode = cfg.getFirstSection("Control");

    while (!secNode.isNull())
    {
        QString type = "";
        cfg.getString(secNode, "Type", type);

        if (type == "Switcher")
        {
            int id = 0;
            cfg.getInt(secNode, "ID", id);

            SwitcherExt ext;

            QString keyNameDec = "";
            cfg.getString(secNode, "KeyNameDec", keyNameDec);
            ext.keyCodeDec = KeySymbolsRRSMap.value(keyNameDec, KEY_Undefined);
            cfg.getString(secNode, "KeyModDec", ext.keyModDecName);

            int num_pos = 2;
            cfg.getInt(secNode, "NumPositions", num_pos);
            ext.numPositions = static_cast<uint16_t>(num_pos);

            switcher_ext.insert(static_cast<uint16_t>(id), ext);
        }

        secNode = cfg.getNextSection();
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherHandler::processKeyInput(const std::set<uint16_t>& pressed_keys,
                                      int cabine_idx, int vehicle_idx)
{
    if (!ctrl_inputs) return;

    for (const auto& [id, _, input] : (*ctrl_inputs)[cabine_idx].getAll())
    {
        if (input.type != "Switcher") continue;

        auto it_ext = switcher_ext.find(id);
        if (it_ext == switcher_ext.end()) continue;

        const auto& ext = it_ext.value();

        // inc: keyCode (из DualKeyHash)
        if (getKeyState(pressed_keys, input.keyCode) &&
            isKeyModifier(pressed_keys, input.keyModOnName))
        {
            sendNextPosition(input.contolledObjectName, id,
                             cabine_idx, vehicle_idx, +1, ext.numPositions);
            hold_state[id] = {+1, 0.0f};
            return;
        }

        // dec: keyCodeDec (из расширения)
        if (getKeyState(pressed_keys, ext.keyCodeDec) &&
            isKeyModifier(pressed_keys, ext.keyModDecName))
        {
            sendNextPosition(input.contolledObjectName, id,
                             cabine_idx, vehicle_idx, -1, ext.numPositions);
            hold_state[id] = {-1, 0.0f};
            return;
        }

        hold_state[id] = {0, 0.0f};
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherHandler::processMouseInput(const io_control_input_t& input,
                                        uint32_t button, bool is_pressed)
{
    if (!is_pressed)
    {
        hold_state[input.id] = {0, 0.0f};
        return;
    }

    int direction = 0;
    if (button == IO_CTRL_LEFT_MOUSE_BUTTON)
        direction = +1;
    else if (button == IO_CTRL_RIGHT_MOUSE_BUTTON)
        direction = -1;
    else
        return;

    auto it_ext = switcher_ext.find(input.id);
    if (it_ext == switcher_ext.end()) return;

    sendNextPosition(input.contolledObjectName, input.id,
                     input.cabine_idx, input.controlled_vehicle_idx,
                     direction, it_ext.value().numPositions);

    hold_state[input.id] = {direction, 0.0f};
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

        // Ищем данные в DualKeyHash
        if (!ctrl_inputs) continue;

        for (size_t cab = 0; cab < ctrl_inputs->size(); ++cab)
        {
            auto io_ctrl = (*ctrl_inputs)[cab].getByKey1(id);
            if (!io_ctrl) continue;

            auto it_ext = switcher_ext.find(id);
            if (it_ext == switcher_ext.end()) break;

            sendNextPosition(io_ctrl->contolledObjectName, id,
                             cab, io_ctrl->controlled_vehicle_idx,
                             state.first, it_ext.value().numPositions);
            state.second = HOLD_DELAY;
            break;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherHandler::sendNextPosition(const QString& objectName,
                                       uint16_t control_id,
                                       int cab_idx, int vehicle_idx,
                                       int direction,
                                       uint16_t num_positions)
{
    if (!ctrl_inputs) return;

    // Текущее положение из feedback
    float current = getSignalValueByName(objectName);

    if (!feedback_signals)
    {
        auto io_ctrl = (*ctrl_inputs)[cab_idx].getByKey1(control_id);
        if (io_ctrl)
            current = io_ctrl->value;
    }

    float step = 1.0f / static_cast<float>(num_positions - 1);
    int cur_idx = static_cast<int>(round(current / step));
    int new_idx = std::clamp(cur_idx + direction, 0,
                             static_cast<int>(num_positions) - 1);

    if (new_idx == cur_idx) return;

    float new_value = static_cast<float>(new_idx) * step;

    // Обновляем состояние в DualKeyHash
    (*ctrl_inputs)[cab_idx].updateByKey1(control_id, new_value);

    // Отправляем
    sendControlSignal(vehicle_idx, cab_idx, control_id, new_value);
}
```

## Интеграция с IOController

`SwitcherHandler` создаётся в `IOController::load_handlers()`, получает
указатель на `io_control_inputs`:

```cpp
// IOController::load_handlers()
if (need_switcher)
{
    SwitcherHandler* sw = new SwitcherHandler(this);
    sw->loadConfig(cfg, cabs_num);   // загружает расширения
    sw->setControlInputs(&io_control_inputs);
    sw->setAnimationSignalsMap(animation_signals_map);
    sw->setFeedbackSignals(feedback_signals);
    connect(sw, &ControlHandler::sigSendControlCommand,
            this, &IOController::sigSendVehicleControlCommand);
    handlers.push_back(sw);
}
```

Остальные вызовы (processKeyInput, processMouseInput, step, setFeedbackSignals)
делегируются через общий список handler-ов — как в `docs/control-handler.md`.

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

```cpp
void SomeDevice::loadConfig(QString cfg_path)
{
    switcher.setControlInputs(&control_inputs);
    switcher.setControlSignalID(60);
}
```

## Поток данных (полный)

```
Viewer (IOController)                Simulator
─────────────────────                ────────

Нажатие Key_Inc (Shift+P):
  SwitcherHandler::processKeyInput()
    читает из DualKeyHash: id=60, keyCode=KEY_P, keyModOnName="Shift"
    читает из расширения: keyCodeDec=KEY_O, numPositions=4
    getSignalValueByName("Controller_Mode") → 0.33
    new_pos = round(0.33/0.33) + 1 → 2
    value = 2 * 0.33 → 0.66
    пишет в DualKeyHash: updateByKey1(60, 0.66)
    sendControlSignal(vehicle_idx, cab_idx, 60, 0.66)
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
  IOController пробрасывает всем handler-ам
    ↓
  SwitcherHandler:
    getSignalValueByName() на следующем нажатии → 0.66
```

## Параметры автоповтора

| Константа | Значение | Описание |
|---|---|---|
| `HOLD_DELAY` | 0.3 сек | Задержка перед началом автоповтора |
| `REPEAT_INTERVAL` | 0.1 сек | Интервал между шагами автоповтора |

## Обратная совместимость

- `SwitcherControl`: если `control_inputs == nullptr` — работает по-старому
- IOController: все изменения в handler-ах, существующие типы Toggle/Button
  не затронуты
- `io_control_input_t`: поля не добавляются

## Влияние на SDK

| Компонент | Файлы | Изменения |
|---|---|---|
| `device` | `switcher-control.h`, `switcher-control.cpp` | Добавление новых методов |
| `io-controller` | `control-handler.h` | Базовый класс `ControlHandler` |
| `io-controller` | `switcher-handler.h`, `switcher-handler.cpp` | Наследник `ControlHandler` |

## План реализации (слайсы)

1. **ControlHandler** — базовый класс с `ctrl_inputs`, вспомогательными методами
2. **ToggleHandler** — вынести Toggle/Button из IOController
3. **IOController** — заполняет DualKeyHash, создаёт handler-ы, делегирует вызовы
4. **SwitcherHandler** — расширения (keyCodeDec, numPositions), inc/dec, автоповтор
5. **VL60IOController** — переходит на `toggle_handler->processToggle()`
6. **SwitcherControl (SDK)** — добавляет чтение из `control_inputs`