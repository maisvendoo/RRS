# Обратная связь (Feedback) для IOController

## Контекст

### Архитектура пользовательского ввода (текущая)

Система ввода разделена на два независимых контура:

**Контур 1 — Viewer (клиент):**
- VSG event system → `InputRouteHandler` → `IOController` (per-vehicle модуль) → TCP → simulator
- VSG event system → `MouseControlHandler` → ray-pick 3D → `IOController` → TCP → simulator
- VSG event system → `UpdateControlToServerHandler` (легаси) → TCP → simulator

**Контур 2 — Simulator (сервер):**
- `FreeJoy` (SFML джойстик) → `control_signals_t` → физика симулятора

Легаси (`UpdateControlToServerHandler`) сохраняется для обратной совместимости
с существующими дополнениями в процессе разработки. `FreeJoy` и аналогичные
устройства в дальнейшем будут работать через TCP/IP-команды единого протокола.

### Единый протокол управления (уже реализован)

**Клиент → Сервер (TCP):**

```
STYPE_SEND_VEHICLE_CONTROL_COMMAND
  int      vehicle_idx    // индекс ПЕ
  int      cab_idx        // индекс кабины
  uint16_t id             // ID сигнала управления
  float    value          // значение сигнала
```

Сериализация: `io_control_input_t::serialize()` (`io-controller-input.h:47`)
Отправка: `TcpClient::sendVehicleControl()` → `STYPE_SEND_VEHICLE_CONTROL_COMMAND`
Приём: `TcpServer::process_client_request()` case `STYPE_SEND_VEHICLE_CONTROL_COMMAND`
(`tcp-server.cpp:323`) → `sigSetVehicleControlCommand(vehicle_idx, cab_idx, id, value)`

### End-to-end: ввод → физика

```
IOController                     Model::slotSetVehicleControlCommand()
  │                                        │
  │ serialize(vehicle_idx, cab_idx, id, value)  │
  ▼                                        ▼
TcpClient::sendVehicleControl()  TcpServer (case STYPE_SEND_VEHICLE_CONTROL_COMMAND)
  │                                        │
  └──────── TCP/IP (port 1992) ────────────┘
                                           │
                                           ▼
                                    vehicle->control_inputs[cab_idx][id] = value
                                           │
                                           ▼
                                    Physics (device signals)
```

## Постановка задачи

Использовать сигналы анимаций (`analogSignal`) из симулятора в качестве обратной
связи для IOController — чтобы читать текущее состояние управляемых контролов по имени
3D-объекта.

В текущей архитектуре IOController хранит локальное состояние контролов, которое
может разойтись с реальным состоянием симулятора (другой игрок, автопилот,
сценарий). 3D-анимации при этом корректно отображают состояние, так как
`ProcAnimation` читает `analogSignal` напрямую.

## Текущая архитектура (детально)

### Поток данных от симулятора к viewer

```
Simulator:
  Vehicle::control_inputs[cab][id]  ←  IOController (TCP)
  Vehicle::analogSignal[signal_id]  ←  Заполняется устройствами (device)

  Model::prepareFeedBack()
    → update_vehicles.vehicles[i].analogSignal = *(vehicle->getAnalogSignals())
    → TCP → Viewer

Viewer:
  VehiclesHandler::slotGetVehiclesStateData()
    → VehiclesHandler::step()
      → VehicleExterior::step(t, dt, &analogSignal)
        → ProcAnimation::setSignals(&analogSignal)
        → ProcAnimation::anim_step()
          → читает (*server_signals)[signal_id]   // signal_id из конфига анимации
```

### Два независимых пространства ID

В симуляторе существуют два отдельных массива сигналов:

```
control_inputs[cab_idx][id]   —  куда IOController пишет команды
                                (id из <Control ID="42">)
        │
        ▼
   Vehicle::control_inputs  (std::vector<QMap<int, float>>)
   Определение: vehicle.h:211

analogSignal[signal_id]        —  что симулятор выдаёт как обратную связь
                                (signal_id из <Animation SignalID="15">)
        │
        ▼
   Vehicle::analogSignal  (std::vector<float>)
   Определение: vehicle.h:335
```

**ID контрола и SignalID анимации — НЕ равны.** Это два разных пространства
идентификаторов. Число `42` из конфига `<Control ID="42">` — это индекс в
`control_inputs`, а не в `analogSignal`. Анимации читают `analogSignal[15]`,
где `15` — это `SignalID` из конфига анимации.

### Маппинг имён (текущий)

- `ProcAnimation` имеет поля: `name` (имя 3D-ноды), `signal_id` (индекс в analogSignal)
- `io_control_input_t` имеет поля: `contolledObjectName` (имя 3D-ноды), `id` (индекс в control_inputs)
- Оба конфига ссылаются на одни и те же 3D-ноды, но разными ID

### Ключевые структуры данных

| Сущность | Файл | Строка | Тип |
|---|---|---|---|
| `Vehicle::control_inputs` | `simulator/vehicle/include/vehicle.h` | 211 | `std::vector<QMap<int, float>>` |
| `Vehicle::analogSignal` | `simulator/vehicle/include/vehicle.h` | 335 | `std::vector<float>` |
| `io_control_input_t` | `viewer/IO-controller/include/io-controller-input.h` | 12 | struct |
| `IOController::io_control_inputs` | `viewer/IO-controller/include/io-controller.h` | 57 | `std::vector<DualKeyHash<uint16_t, QString, io_control_input_t>>` |
| `ProcAnimation::signal_id` | `viewer/include/ProcAnimation.h` | 36 | `std::int32_t` |
| `ProcAnimation::name` | `viewer/include/ProcAnimation.h` | 33 | `std::string` |
| `animations_t::animations` | `viewer/include/animations-list.h` | 21 | `std::multimap<std::int32_t, vsg::ref_ptr<ProcAnimation>>` |
| `VehicleExterior::animated_nodes` | `viewer/include/VehicleExterior.h` | 37 | `std::vector<vsg::ref_ptr<AnimatedPagedLOD>>` |
| `VehicleExterior::io_controller` | `viewer/include/VehicleExterior.h` | 51 | `IOController*` |

## Предлагаемое решение

### Принцип

Строить маппинг **имя 3D-объекта → signal_id** из конфигов анимаций (где
`ProcAnimation::name` + `ProcAnimation::signal_id` уже заданы), а НЕ из контролов
IOController. Поиск сигнала выполняется по этому маппингу, независимо от `ID` контрола.

`analogSignal` (тот же `std::vector<float>`, что уже приходит в viewer для
анимаций) прокидвается в IOController как источник Feedback-сигналов.

### Изменения в IOController

**Файл:** `viewer/IO-controller/include/io-controller.h`

Добавить в public:

```cpp
/// Установить карту сигналов анимаций (имя 3D-объекта → signal_id)
void setAnimationSignalsMap(const QMap<QString, uint16_t>& map);

/// Установить указатель на массив сигналов обратной связи от симулятора
void setFeedbackSignals(const std::vector<float>* signals);

/// Получить текущее значение сигнала по имени 3D-объекта
float getSignalValue(const QString& objectName) const;
```

Добавить в private:

```cpp
/// Маппинг: имя 3D-объекта → ID сигнала обратной связи из analogSignal
QMap<QString, uint16_t> animation_signals_map;

/// Указатель на массив аналоговых сигналов от симулятора
const std::vector<float>* feedback_signals = nullptr;
```

**Файл:** `viewer/IO-controller/src/io-controller.cpp`

Реализация методов:

```cpp
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::setAnimationSignalsMap(const QMap<QString, uint16_t>& map)
{
    animation_signals_map = map;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IOController::setFeedbackSignals(const std::vector<float>* signals)
{
    feedback_signals = signals;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float IOController::getSignalValue(const QString& objectName) const
{
    if (objectName.isEmpty() || !feedback_signals)
        return 0.0f;

    // Поиск по точному имени
    auto it = animation_signals_map.find(objectName);
    if (it != animation_signals_map.end())
    {
        uint16_t signal_id = it.value();
        if (signal_id < feedback_signals->size())
            return (*feedback_signals)[signal_id];
    }

    // Fallback: поиск с endsWith (для совместимости)
    for (auto it = animation_signals_map.begin(); it != animation_signals_map.end(); ++it)
    {
        if (objectName.endsWith(it.key()))
        {
            if (it.value() < feedback_signals->size())
                return (*feedback_signals)[it.value()];
        }
    }

    return 0.0f;
}
```

### Изменения в VehicleExterior

**Файл:** `viewer/src/VehicleExterior.cpp`

В `loadVehicle()`, после загрузки всех моделей и анимаций (после `load_models()`
и `load_io_controller_module()`):

```cpp
// Строим карту имя_объекта → signal_id из конфигов анимаций
QMap<QString, uint16_t> anim_signals_map;
for (const auto& animated_pagedLOD : animated_nodes)
{
    for (const auto& [signal_id, animation] : animated_pagedLOD->animations_map->animations)
    {
        if (!animation->name.empty())
        {
            anim_signals_map.insert(
                QString::fromStdString(animation->name),
                static_cast<uint16_t>(signal_id));
        }
    }
}

if (io_controller)
    io_controller->setAnimationSignalsMap(anim_signals_map);
```

В `VehicleExterior::step(float t, float dt, std::vector<float>* server_signals)`:

```cpp
void VehicleExterior::step(float t, float dt, std::vector<float>* server_signals)
{
    // Прокидываем сигналы обратной связи в IOController
    if (io_controller)
        io_controller->setFeedbackSignals(server_signals);

    // Анимации (без изменений)
    for (const auto& animated_pagedLOD : animated_nodes)
    {
        if (animated_pagedLOD->children[0].node)
        {
            for (const auto& [signal_id, animation] : animated_pagedLOD->animations_map->animations)
            {
                animation->setSignals(server_signals);
                animation->step(t, dt);
            }
        }
    }
}
```

### Цепочка поиска (итоговая)

```
Имя 3D-объекта ("BrakeCrane_Handle")
       │
       ▼
animation_signals_map["BrakeCrane_Handle"] = 15  (SignalID из анимации)
       │
       ▼
feedback_signals[15] = 0.75  (реальное значение из физики)
       │
       ▼
IOController::getSignalValue("BrakeCrane_Handle") → 0.75
```

Независимо от того, какой `ID` у контрола в IOController (хоть 42, хоть 999) —
сигнал читается из `analogSignal[signal_id_анимации]`.

### Использование в Toggle-контролах

При обработке тумблера (`processTumbler`, `mouseProcessTumbler`) текущее состояние
читать через `getSignalValue(io_ctrl.contolledObjectName)`, а не из локального
`io_ctrl->value`:

```cpp
void IOController::processTumbler(size_t cab_idx,
                                  const uint16_t &control_id,
                                  const std::set<uint16_t> &pressed_keys)
{
    auto io_ctrl = io_control_inputs[cab_idx].getByKey1(control_id);
    if (!io_ctrl) return;

    if (getKeyState(pressed_keys, io_ctrl->keyCode))
    {
        // Текущее состояние из обратной связи
        float current_value = getSignalValue(io_ctrl->contolledObjectName);
        // fallback, если нет feedback
        if (!feedback_signals)
            current_value = io_ctrl->value;

        if (io_ctrl->keyModOnName == io_ctrl->keyModOffName)
        {
            if (checkModKey(io_ctrl->keyModOnName, pressed_keys))
            {
                // Инвертируем относительно реального состояния
                io_ctrl->value = 1.0f - current_value;
                io_control_inputs[cab_idx].updateByKey1(control_id, io_ctrl.value());
                emit sigSendVehicleControlCommand(io_ctrl->serialize());
                return;
            }
        }

        if (checkModKey(io_ctrl->keyModOnName, pressed_keys))
        {
            io_ctrl->value = 1.0f;
            io_control_inputs[cab_idx].updateByKey1(control_id, io_ctrl.value());
            emit sigSendVehicleControlCommand(io_ctrl->serialize());
            return;
        }

        if (checkModKey(io_ctrl->keyModOffName, pressed_keys))
        {
            io_ctrl->value = 0.0f;
            io_control_inputs[cab_idx].updateByKey1(control_id, io_ctrl.value());
            emit sigSendVehicleControlCommand(io_ctrl->serialize());
            return;
        }
    }
}
```

```cpp
void IOController::mouseProcessTumbler(io_control_input_t input,
                                       uint32_t button,
                                       bool is_pressed)
{
    auto io_ctrl = io_control_inputs[input.cabine_idx].getByKey1(input.id);
    if (!io_ctrl) return;

    if (input.type == "Toggle")
    {
        // Читаем текущее состояние из обратной связи
        float current_value = getSignalValue(io_ctrl->contolledObjectName);
        if (!feedback_signals)
            current_value = io_ctrl->value;

        if (button == IO_CTRL_LEFT_MOUSE_BUTTON)
        {
            io_ctrl->value = 1.0f - current_value;
            io_control_inputs[input.cabine_idx].updateByKey1(input.id, io_ctrl.value());
            emit sigSendVehicleControlCommand(io_ctrl->serialize());
        }

        if (button == IO_CTRL_RIGHT_MOUSE_BUTTON && current_value > 0.5f)
        {
            io_ctrl->value = 0.0f;
            io_control_inputs[input.cabine_idx].updateByKey1(input.id, io_ctrl.value());
            emit sigSendVehicleControlCommand(io_ctrl->serialize());
        }
    }
}
```

### Схема данных (обновлённая)

```
Simulator                      Viewer
───────                        ──────
Vehicle::analogSignal[]
       │
       ▼
TCP ───────→ VehiclesHandler::slotGetVehiclesStateData()
                                │
                                ▼
                    VehiclesHandler::step()
                                │
                                ▼
                    VehicleExterior::step(t, dt, &analogSignal)
                      ├── io_controller->setFeedbackSignals(&analogSignal)  [NEW]
                      └── animation->setSignals(&analogSignal)
                           animation->step()  ← читает analogSignal[signal_id]
```

### Ключевой нюанс: два пространства ID

При разработке важно помнить: `control_inputs[id]` и `analogSignal[signal_id]` —
это разные массивы с разными системами индексации. IOController при отправке
команд использует `id` (индекс в `control_inputs`). При чтении обратной связи —
`signal_id` (индекс в `analogSignal`), полученный из маппинга анимаций.

Связующим звеном является имя 3D-объекта: оно одинаково в обоих конфигах
(IOController config: `<ObjectNameCab1>`, Animation config: `name`).

### Загрузка маппинга — однократно при старте

Маппинг `имя → signal_id` строится один раз в `VehicleExterior::loadVehicle()`,
после загрузки всех моделей и анимаций. Пересборка маппинга в каждом кадре
не требуется и была бы неэффективна.

`VehicleExterior::step()` в каждом кадре обновляет только указатель на
`analogSignal` (через `setFeedbackSignals`), что является операцией O(1).

## Влияние на SDK

`io-controller` входит в SDK (устанавливается командой `make install`).
Изменения:
- Добавление новых методов (`setAnimationSignalsMap`, `setFeedbackSignals`,
  `getSignalValue`) — обратно совместимо, существующий ABI не ломается
- Изменение поведения `processTumbler()` и `mouseProcessTumbler()` — меняется
  логика для всех наследников, не переопределяющих `keysProcess`

Остальные изменения (`VehicleExterior`) — не входят в SDK.

## План реализации (слайсы)

### Слайс 1: IOController — добавить приём feedback signals

**Файлы:**
- `viewer/IO-controller/include/io-controller.h`
- `viewer/IO-controller/src/io-controller.cpp`

**Изменения:**
- Добавить член `animation_signals_map` (`QMap<QString, uint16_t>`)
- Добавить член `feedback_signals` (`const std::vector<float>*`)
- Добавить метод `setAnimationSignalsMap(const QMap<QString, uint16_t>&)`
- Добавить метод `setFeedbackSignals(const std::vector<float>*)`
- Добавить метод `getSignalValue(const QString&)`

**Влияние на SDK:** Да, обратно совместимо.

### Слайс 2: VehicleExterior — построить карту анимаций и прокинуть сигналы

**Файлы:**
- `viewer/src/VehicleExterior.cpp`

**Изменения:**
- В `loadVehicle()` после загрузки анимаций: построить `anim_signals_map`
  из `animated_nodes[i]->animations_map->animations[signal_id]->name`
- Вызвать `io_controller->setAnimationSignalsMap(anim_signals_map)`
- В `step(t, dt, server_signals)`: вызвать `io_controller->setFeedbackSignals(server_signals)`

**Влияние на SDK:** Нет.

### Слайс 3: Toggle-контролы — использовать feedback

**Файлы:**
- `viewer/IO-controller/src/io-controller.cpp`

**Изменения:**
- В `processTumbler()`: читать `current_value = getSignalValue(contolledObjectName)`
- В `mouseProcessTumbler()`: читать `current_value = getSignalValue(contolledObjectName)`

**Влияние на SDK:** Да, меняется поведение базового класса для Toggle-контролов.

### Требование к последовательности

Слайсы должны выполняться последовательно — каждый последующий опирается
на изменения предыдущего. После каждого слайса — коммит и пуш в удалённый
репозиторий. Никакого слияния веток без явного указания.