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

## Формат конфига

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

Параметры:

| Параметр | Описание |
|---|---|
| `type="Switcher"` | Тип контрола — многопозиционный переключатель |
| `ID` | Идентификатор сигнала управления в `control_inputs` |
| `KeyNameInc` | Клавиша увеличения позиции (вперёд) |
| `KeyModInc` | Модификатор увеличения (опционально) |
| `KeyNameDec` | Клавиша уменьшения позиции (назад) |
| `KeyModDec` | Модификатор уменьшения (опционально) |
| `NumPositions` | Количество позиций переключателя |
| `ObjectNameCab1` | Имя 3D-объекта для мышиного ввода |

## Изменения в IOController

### 1. Расширение `io_control_input_t`

**Файл:** `viewer/IO-controller/include/io-controller-input.h`

Добавить поля для Switcher:

```cpp
struct io_control_input_t
{
    // ... существующие поля ...

    // Для Switcher
    uint16_t keyCodeInc = 0;          ///< Клавиша увеличения позиции
    QString  keyModIncName = "";      ///< Модификатор увеличения
    uint16_t keyCodeDec = 0;          ///< Клавиша уменьшения позиции
    QString  keyModDecName = "";      ///< Модификатор уменьшения
    uint16_t numPositions = 2;        ///< Количество позиций

    // ... serialize(), deserialize() — без изменений,
    //     серверу как и прежде передаём только controlled_vehicle_idx,
    //     cabine_idx, id, value (положение)
};
```

### 2. Добавить состояние удержания в IOController

**Файл:** `viewer/IO-controller/include/io-controller.h`

В `private:` добавить:

```cpp
/// Параметры автоповтора при удержании
static constexpr float SWITCHER_HOLD_DELAY    = 0.3f;  // задержка перед автоповтором, сек
static constexpr float SWITCHER_REPEAT_INTERVAL = 0.1f; // интервал автоповтора, сек

/// Состояние удержания switcher: control_id → (направление, время_удержания)
/// направление: +1 = inc, -1 = dec, 0 = нет удержания
QMap<uint16_t, std::pair<int, float>> switcher_hold_state;
```

В `protected:` добавить виртуальный метод (для кастомных модулей):

```cpp
/// Обработка переключателя типа Switcher
void processSwitcher(size_t cab_idx, const uint16_t& control_id,
                     const std::set<uint16_t>& pressed_keys);
```

### 3. Загрузка конфига Switcher

**Файл:** `viewer/IO-controller/src/io-controller.cpp`

В `load_config()`, в цикле по секциям `<Control>`, после загрузки
общих полей добавить:

```cpp
if (ic_input.type == "Switcher")
{
    QString keyNameInc = "";
    cfg.getString(secNode, "KeyNameInc", keyNameInc);
    ic_input.keyCodeInc = KeySymbolsRRSMap.value(keyNameInc, KEY_Undefined);
    cfg.getString(secNode, "KeyModInc", ic_input.keyModIncName);

    QString keyNameDec = "";
    cfg.getString(secNode, "KeyNameDec", keyNameDec);
    ic_input.keyCodeDec = KeySymbolsRRSMap.value(keyNameDec, KEY_Undefined);
    cfg.getString(secNode, "KeyModDec", ic_input.keyModDecName);

    int num_pos = 2;
    cfg.getInt(secNode, "NumPositions", num_pos);
    ic_input.numPositions = static_cast<uint16_t>(num_pos);

    getUsageString(ic_input); // переопределить для Switcher

    // Вставка в DualKeyHash — без изменений, вставляется как обычно
    // по id и ObjectName. keyCodeInc/keyCodeDec используются в processSwitcher().
}
```

Дополнить `getUsageString()`:

```cpp
if (ic_input.type == "Switcher")
{
    ic_input.usage = QString("Вперёд: %1 | Назад: %2")
                        .arg(ic_input.keyModIncName + "+" + keyNameInc.mid(4))
                        .arg(ic_input.keyModDecName + "+" + keyNameDec.mid(4));
}
```

### 4. Обработка нажатий клавиш для Switcher

**Файл:** `viewer/IO-controller/src/io-controller.cpp`

В `processKeyBoardInput()`, после существующей обработки Toggle/Button,
добавить проверку Switcher. Либо в `keysProcess()` вызывать
`processSwitcher()` для каждого контрола типа Switcher.

Новый метод:

```cpp
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::processSwitcher(size_t cab_idx,
                                   const uint16_t& control_id,
                                   const std::set<uint16_t>& pressed_keys)
{
    auto io_ctrl = io_control_inputs[cab_idx].getByKey1(control_id);
    if (!io_ctrl) return;

    if (io_ctrl->type != "Switcher") return;

    // Проверка нажатия клавиши увеличения
    if (getKeyState(pressed_keys, io_ctrl->keyCodeInc) &&
        checkModKey(io_ctrl->keyModIncName, pressed_keys))
    {
        // Отправка +1 позиция
        sendSwitcherPosition(cab_idx, *io_ctrl, +1);
        switcher_hold_state[control_id] = {+1, 0.0f};
        return;
    }

    // Проверка нажатия клавиши уменьшения
    if (getKeyState(pressed_keys, io_ctrl->keyCodeDec) &&
        checkModKey(io_ctrl->keyModDecName, pressed_keys))
    {
        sendSwitcherPosition(cab_idx, *io_ctrl, -1);
        switcher_hold_state[control_id] = {-1, 0.0f};
        return;
    }

    // Ни одна клавиша не нажата — сброс удержания
    switcher_hold_state[control_id] = {0, 0.0f};
}
```

Вспомогательный метод отправки положения:

```cpp
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::sendSwitcherPosition(size_t cab_idx,
                                        io_control_input_t& io_ctrl,
                                        int direction)
{
    // Текущее положение из feedback (0.0–1.0)
    float current = getSignalValueByName(io_ctrl.contolledObjectName);
    if (!feedback_signals)
        current = io_ctrl.value;

    // Шаг в долях
    float step = 1.0f / static_cast<float>(io_ctrl.numPositions - 1);

    // Новая позиция с ограничением
    int cur_idx = static_cast<int>(round(current / step));
    int new_idx = std::clamp(cur_idx + direction, 0,
                             static_cast<int>(io_ctrl.numPositions) - 1);

    if (new_idx == cur_idx) return; // уже в крайнем положении

    io_ctrl.value = static_cast<float>(new_idx) * step;
    io_control_inputs[cab_idx].updateByKey1(io_ctrl.id, io_ctrl.value);
    emit sigSendVehicleControlCommand(io_ctrl.serialize());
}
```

### 5. Обработка удержания в `step()`

**Файл:** `viewer/IO-controller/src/io-controller.cpp`

В `step()` добавить:

```cpp
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::step(float t, float dt)
{
    // Обработка удержания клавиш Switcher
    for (auto& [control_id, state] : switcher_hold_state)
    {
        if (state.first == 0) continue;

        state.second += dt;

        // Задержка перед автоповтором
        if (state.second < SWITCHER_HOLD_DELAY)
            continue;

        // Интервал автоповтора
        if (state.second < SWITCHER_HOLD_DELAY + SWITCHER_REPEAT_INTERVAL)
            continue;

        // Ищем контрол и отправляем следующий шаг
        for (size_t cab = 0; cab < io_control_inputs.size(); ++cab)
        {
            auto io_ctrl = io_control_inputs[cab].getByKey1(control_id);
            if (!io_ctrl) continue;

            sendSwitcherPosition(cab, *io_ctrl, state.first);
            state.second = SWITCHER_HOLD_DELAY; // сброс на начало интервала
            break;
        }
    }
}
```

### 6. Мышиный ввод для Switcher

**Файл:** `viewer/IO-controller/src/io-controller.cpp`

В `mouseProcessTumbler()` добавить обработку Switcher:

```cpp
void IOController::mouseProcessTumbler(io_control_input_t input,
                                       uint32_t button,
                                       bool is_pressed)
{
    auto io_ctrl = io_control_inputs[input.cabine_idx].getByKey1(input.id);
    if (!io_ctrl) return;

    if (input.type == "Toggle")
    {
        // ... существующая обработка Toggle ...
    }

    if (input.type == "Switcher")
    {
        if (is_pressed)
        {
            int direction = 0;
            if (button == IO_CTRL_LEFT_MOUSE_BUTTON)
                direction = +1;
            else if (button == IO_CTRL_RIGHT_MOUSE_BUTTON)
                direction = -1;
            else
                return;

            sendSwitcherPosition(input.cabine_idx, *io_ctrl, direction);
            switcher_hold_state[input.id] = {direction, 0.0f};
        }
        else
        {
            // Кнопка отпущена — сброс удержания
            switcher_hold_state[input.id] = {0, 0.0f};
        }
    }
}
```

### 7. Сброс удержания при отпускании клавиш

В `setReleasedKey()`, при отпускании клавиши, нужно проверить, не
относится ли она к активному Switcher, и сбросить удержание:

```cpp
void IOController::setReleasedKey(uint16_t keyBase)
{
    _pressed_keys.erase(keyBase);

    // Проверяем, не была ли отпущена клавиша активного Switcher
    for (auto& [control_id, state] : switcher_hold_state)
    {
        if (state.first == 0) continue;

        for (size_t cab = 0; cab < io_control_inputs.size(); ++cab)
        {
            auto io_ctrl = io_control_inputs[cab].getByKey1(control_id);
            if (!io_ctrl) continue;

            if (keyBase == io_ctrl->keyCodeInc || keyBase == io_ctrl->keyCodeDec)
            {
                state = {0, 0.0f};
                break;
            }
        }
    }

    processKeyBoardInput();
}
```

## Изменения в SwitcherControl (SDK)

### 1. Новые поля и методы

**Файл:** `simulator/device/include/switcher-control.h`

Добавить в public:

```cpp
/// Задать указатель на массив управляющих сигналов от ТС
void setControlInputs(QMap<int, float>* inputs);

/// Задать ID сигнала управления положением
void setControlSignalID(uint16_t signal_id);
```

Добавить в protected:

```cpp
/// Указатель на массив управляющих сигналов от Vehicle
QMap<int, float>* control_inputs = nullptr;

/// ID сигнала управления положением
uint16_t control_signal_id = KEY_Undefined;
```

### 2. Модификация `step()`

**Файл:** `simulator/device/src/switcher-control.cpp`

```cpp
bool SwitcherControl::step(double t, double dt)
{
    bool changed = false;

    // ─── Новый режим: управление положением из control_inputs ───
    if (control_inputs && control_signal_id != KEY_Undefined)
    {
        float target = control_inputs->value(control_signal_id, getHandlePosition());
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
        // ... существующая логика без изменений ...
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

        // Spring return внутри легаси-ветки
        if (allow_spring_first && (state == 0))
            changed = incPos() || changed;
        if (allow_spring_last && (state == (num_states - 1)))
            changed = decPos() || changed;

        return changed;
    }

    // ─── Spring return (для нового режима и режима без клавиш) ───
    if (is_spring_first && state == 0)
        changed = incPos() || changed;

    if (is_spring_last && state == num_states - 1)
        changed = decPos() || changed;

    return changed;
}
```

### 3. Реализация новых методов

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

В `step()` устройства вызов `switcher.step(t, dt)` уже обработает и
положение из `control_inputs`, и spring return.

## Поток данных (полный)

```
Viewer (IOController)                Simulator
─────────────────────                ────────

Нажатие Key_Inc (Shift+P):
  getSignalValueByName("Controller_Mode") → 0.33
  new_pos = round(0.33/0.33) + 1 → 2
  value = 2 * 0.33 → 0.66
  send(vehicle_idx, cab_idx, 60, 0.66)
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
    ProcAnimation → analogSignal[signal_id]
    ← getSignalValueByName() вернёт 0.66
```

## Параметры автоповтора

| Константа | Значение | Описание |
|---|---|---|
| `SWITCHER_HOLD_DELAY` | 0.3 сек | Задержка перед началом автоповтора |
| `SWITCHER_REPEAT_INTERVAL` | 0.1 сек | Интервал между шагами автоповтора |

## Обратная совместимость

- `SwitcherControl`: если `control_inputs == nullptr` — работает по-старому
  через `pressed_keys`
- IOController: если тип не `"Switcher"` — поведение не изменяется
- `io_control_input_t`: новые поля имеют значения по умолчанию
  (keyCodeInc/Dec = 0, numPositions = 2)

## Влияние на SDK

| Компонент | Файлы | Изменения |
|---|---|---|
| `device` | `switcher-control.h`, `switcher-control.cpp` | Добавление новых методов (обратно совместимо) |
| `io-controller` | `io-controller.h`, `io-controller-input.h`, `io-controller.cpp` | Новый тип контрола (обратно совместимо) |

## План реализации (слайсы)

1. **IOController — база Switcher:** добавить поля в `io_control_input_t`,
   загрузку конфига, `processSwitcher()`, `sendSwitcherPosition()`
2. **IOController — удержание:** `step()` с автоповтором, сброс в
   `setReleasedKey()`
3. **IOController — мышь:** обработка Switcher в `mouseProcessTumbler()`
4. **SwitcherControl (SDK):** добавить `control_inputs`, `control_signal_id`,
   модифицировать `step()`