# Управление Switcher через IOController

## Контекст

`Switcher` и `SwitcherControl` — классы SDK (компонент `device`), реализующие
многопозиционные переключатели с поддержкой звука переключения и автовозврата
из крайних положений (spring return).

Новая архитектура: IOController отправляет **абсолютное положение**
переключателя (0.0–1.0) через единый TCP-протокол. На стороне симулятора
положение пишется в `Vehicle::control_inputs[cab_idx][id]`, откуда его читает
`SwitcherControl` в `step()`.

## Принцип

- **Один сигнальный ID** на орган управления. IOController отправляет
  положение (float 0.0–1.0), а не приращение.
- **Feedback** — IOController читает текущее положение из `analogSignal`
  через `getSignalValue()`.
- **Удержание клавиши/кнопки** — автоповтор с задержкой и интервалом.
- **Состояние** хранится в полях `SwitcherHandler` (наследник `ControlHandler`).
  handler сам управляет своими полями, хэш хранит указатель на него.

## Формат конфига

```xml
<Control>
    <ID>60</ID>
    <Name>Контроллер режимов</Name>
    <Type>Switcher</Type>
    <value1>0.0</value1>
    <value2>0.0</value2>
    <KeyNameInc>KEY_P</KeyNameInc>
    <KeyModIncName>Shift</KeyModIncName>
    <KeyNameDec>KEY_O</KeyNameDec>
    <KeyModDecName>Ctrl</KeyModDecName>
    <NumPositions>4</NumPositions>
    <MinValue>0.0</MinValue>
    <MaxValue>1.0</MaxValue>
    <ObjectNameCab1>Crane_Selector</ObjectNameCab1>
    <ObjectNameCab2>Crane_Selector</ObjectNameCab2>
</Control>
```

### Поля

| Поле | Описание |
|------|----------|
| `Type` | `"Switcher"` — тип контрола |
| `ID` | Идентификатор сигнала управления |
| `KeyNameInc` | Клавиша увеличения позиции |
| `KeyModIncName` | Модификатор увеличения (опционально) |
| `KeyNameDec` | Клавиша уменьшения позиции |
| `KeyModDecName` | Модификатор уменьшения (опционально) |
| `NumPositions` | Количество позиций (>= 2) |
| `MinValue` | Минимальное значение (по умолч. 0.0) |
| `MaxValue` | Максимальное значение (по умолч. 1.0) |
| `ObjectNameCab{1,2}` | Имя 3D-объекта для мышиного ввода |

Модификаторы опциональны — если поле пустое, клавиша срабатывает без модификатора.

## Класс SwitcherHandler

Наследует `ControlHandler`. Управляет одной сущностью — собой (не итерирует хэш).

### Поля

```cpp
class SwitcherHandler : public ControlHandler
{
    // унаследовано от ControlHandler:
    // id, value, controlled_vehicle_idx, cabine_idx, contolledObjectName, name

    uint16_t keyCodeInc = 0;          // клавиша INC
    QString  keyModIncName = "";      // модификатор INC
    uint16_t keyCodeDec = 0;          // клавиша DEC
    QString  keyModDecName = "";      // модификатор DEC
    uint16_t numPositions = 2;
    float minValue = 0.0f;
    float maxValue = 1.0f;        // число позиций

    int    hold_direction = 0;         // направление автоповтора
    float  hold_time = 0.0f;           // таймер автоповтора

    static constexpr float HOLD_DELAY      = 0.3f;
    static constexpr float REPEAT_INTERVAL = 0.1f;
};
```

### load_config

```cpp
bool SwitcherHandler::load_config(CfgReader &cfg, QDomNode secNode)
{
    ControlHandler::load_config(cfg, secNode);  // Name, Description, ID, KeyName, KeyMod*

    QString keyInc;
    cfg.getString(secNode, "KeyNameInc", keyInc);
    keyCodeInc = KeySymbolsRRSMap.value(keyInc, KEY_Undefined);
    cfg.getString(secNode, "KeyModIncName", keyModIncName);

    QString keyDec;
    cfg.getString(secNode, "KeyNameDec", keyDec);
    keyCodeDec = KeySymbolsRRSMap.value(keyDec, KEY_Undefined);
    cfg.getString(secNode, "KeyModDecName", keyModDecName);

    int np = 2;
    cfg.getInt(secNode, "NumPositions", np);
    numPositions = static_cast<uint16_t>(std::max(np, 2));

    return true;
}
```

### processKeyInput

```cpp
void SwitcherHandler::processKeyInput(const std::set<uint16_t> &pk)
{
    if (getKeyState(pk, keyCodeInc) && isKeyModifier(pk, keyModIncName))
    {
        sendNextPosition(+1);
        hold_direction = +1; hold_time = 0;
        return;
    }

    if (getKeyState(pk, keyCodeDec) && isKeyModifier(pk, keyModDecName))
    {
        sendNextPosition(-1);
        hold_direction = -1; hold_time = 0;
        return;
    }

    hold_direction = 0;
    hold_time = 0;
}
```

### processMouseInput

```cpp
void SwitcherHandler::processMouseInput(uint32_t button, bool is_pressed)
{
    if (!is_pressed) { hold_direction = 0; hold_time = 0; return; }

    int dir = 0;
    if (button == CTRL_LEFT_MOUSE_BUTTON)  dir = +1;
    if (button == CTRL_RIGHT_MOUSE_BUTTON) dir = -1;
    if (dir == 0) return;

    sendNextPosition(dir);
    hold_direction = dir;
    hold_time = 0;
}
```

### sendNextPosition (приватный)

```cpp
void SwitcherHandler::sendNextPosition(int direction)
{
    // Текущее положение: с сервера (feedback) или из своего поля
    float cur = getSignalValue();
    if (feedback_signals == nullptr) cur = value;

    float range = maxValue - minValue;
    float step = range / (numPositions - 1);
    int idx = static_cast<int>(std::round((cur - minValue) / step));
    int new_idx = std::clamp(idx + direction, 0, numPositions - 1);
    if (new_idx == idx) return;

    value = minValue + static_cast<float>(new_idx) * step;
    sendControlSignal();
}
```

### step (автоповтор)

```cpp
void SwitcherHandler::step(float t, float dt)
{
    if (hold_direction == 0) return;

    hold_time += dt;
    if (hold_time < HOLD_DELAY) return;

    hold_time -= REPEAT_INTERVAL;
    sendNextPosition(hold_direction);
}
```

### getUsage

```cpp
QString SwitcherHandler::getUsage() const
{
    return QString("ЛКМ — вперёд | ПКМ — назад");
}
```

## Регистрация в IOController

В `create_handler` (виртуальный метод, переопределяется в VL60IOController):

```cpp
ControlHandler *IOController::create_handler(const QString &type,
                                             QDomNode secNode, CfgReader &cfg)
{
    if (type == "Toggle")   return new ToggleHandler();
    if (type == "Button")   return new ButtonHandler();
    if (type == "Switcher") return new SwitcherHandler();
    return nullptr;
}
```

`load_config` в базовом `IOController` автоматически вызовет `setAnimationSignalsMap`,
`connect` сигнала и вставку в хэш — handler'у ничего дополнительно не нужно.

## Поток данных (полный)

```
Viewer (IOController)                Simulator
─────────────────────                ────────

Нажатие KeyInc (Shift+P):
  SwitcherHandler::processKeyInput()
    getSignalValue() → 0.33          (или value, если feedback нет)
    new_idx = round(0.33/0.33) + 1 → 2
    value = 2 * 0.33 → 0.66
    sendControlSignal()
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
                           prepareFeedback()
                             analogSignal[signal_id_анимации] = 0.66
       │
       ▼ TCP
Viewer:
  IOController::step()
    handler->setFeedbackSignals(&analogSignal)
    handler->step(t, dt)              // автоповтор
    ↓
  SwitcherHandler:
    getSignalValue() на следующем нажатии → 0.66
```

## Параметры автоповтора

| Константа | Значение | Описание |
|-----------|----------|----------|
| `HOLD_DELAY` | 0.3 сек | Задержка перед началом автоповтора |
| `REPEAT_INTERVAL` | 0.1 сек | Интервал между шагами автоповтора |

## Обратная совместимость

- `SwitcherControl` в SDK: если `control_inputs == nullptr` — работает по-старому
- IOController: все изменения в handler-ах, существующие типы Toggle/Button не затронуты
- `create_handler` — виртуальный, аддоны переопределяют без правки базового класса