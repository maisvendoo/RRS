# ControlHandler — базовая архитектура обработчиков управления

## Мотивация

IOController исторически монолитен: вся логика Toggle, Button и вновь
добавляемого Switcher живёт в одном классе. Это затрудняет расширение.

Решение: выделить базовый класс `ControlHandler` и для каждого типа контрола
создать отдельного наследника. IOController оперирует списком `ControlHandler*`,
делегируя им ввод и шаг симуляции.

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

    /// Указатель на массив аналоговых сигналов от симулятора
    const std::vector<float>* feedback_signals = nullptr;

    /// Маппинг: имя 3D-объекта → ID сигнала обратной связи
    QMap<QString, uint16_t> animation_signals_map;

    /// Получить значение сигнала по имени объекта
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

        // Fallback: endsWith
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

    bool loadConfig(CfgReader &cfg, int cabs_num) override;

    void processKeyInput(const std::set<uint16_t>& pressed_keys,
                         int cabine_idx, int vehicle_idx) override;

    void processMouseInput(const io_control_input_t& input,
                           uint32_t button, bool is_pressed) override;

private:

    struct ToggleData
    {
        uint16_t id = 0;
        uint16_t keyCode = 0;
        QString keyModOnName = "";
        QString keyModOffName = "";
        float value = 0.0f;
        QString objectName = "";
        int cabine_idx = 0;
        int vehicle_idx = 0;
    };

    std::vector<QMap<uint16_t, ToggleData>> toggles;

    void processToggle(size_t cab_idx, const uint16_t& control_id,
                       const std::set<uint16_t>& pressed_keys);

    void processButton(size_t cab_idx, const uint16_t& control_id,
                       const std::set<uint16_t>& pressed_keys);

    ToggleData* findById(int cab_idx, uint16_t id);
    bool findByName(const QString& name, ToggleData& out) const;
};

#endif // TOGGLE_HANDLER_H
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

    std::vector<DualKeyHash<uint16_t, QString, io_control_input_t>> io_control_inputs;
    int cabs_num = 0;

    virtual void keysProcess(std::set<uint16_t> &pressed_keys);
    virtual void processMouseInput(io_control_input_t input, uint32_t button, bool is_pressed);

private:
    /// Обработчики всех типов контролов
    std::vector<ControlHandler*> handlers;

    QMap<QString, uint16_t> animation_signals_map;
    const std::vector<float>* feedback_signals = nullptr;

    void load_handlers(CfgReader &cfg);
    void processKeyBoardInput();
};

// Загрузка модуля
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

    // Загружаем все контролы (для мышиного поиска)
    auto secNode = cfg.getFirstSection("Control");
    while (!secNode.isNull())
    {
        io_control_input_t ic_input;
        // ... общая загрузка id, value1, value2, ObjectName ...
        // Вставка в DualKeyHash
        ...

        secNode = cfg.getNextSection();
    }

    // Создаём обработчики
    load_handlers(cfg);

    return true;
}

void IOController::load_handlers(CfgReader &cfg)
{
    auto secNode = cfg.getFirstSection("Control");

    while (!secNode.isNull())
    {
        QString type = "";
        cfg.getString(secNode, "Type", type);

        ControlHandler* handler = nullptr;

        if (type == "Toggle" || type == "Button")
        {
            handler = new ToggleHandler(this);
        }
        else if (type == "Switcher")
        {
            // Подключаемся к сигналу
            SwitcherHandler* sw = new SwitcherHandler(this);
            handler = sw;
        }

        if (handler)
        {
            handler->loadConfig(cfg, cabs_num);
            handler->setAnimationSignalsMap(animation_signals_map);
            handler->setFeedbackSignals(feedback_signals);

            connect(handler, &ControlHandler::sigSendControlCommand,
                    this, &IOController::sigSendVehicleControlCommand);

            handlers.push_back(handler);
        }

        secNode = cfg.getNextSection();
    }
}
```

Остальные методы делегируют вызовы всем handler-ам:

```cpp
void IOController::processKeyBoardInput()
{
    // ... существующая фильтрация pressed_keys ...

    for (auto* handler : handlers)
        handler->processKeyInput(pressed_keys, cabine_idx, vehicle_idx);

    keysProcess(pressed_keys);
}

void IOController::mouseInputProcess(io_control_input_t input, uint32_t button, bool is_pressed)
{
    for (auto* handler : handlers)
        handler→processMouseInput(input, button, is_pressed);

    processMouseInput(input, button, is_pressed);
}

void IOController::step(float t, float dt)
{
    for (auto* handler : handlers)
        handler→step(dt);
}

void IOController::setFeedbackSignals(const std::vector<float>* signals){
    feedback_signals = sign als;
    for (auto* handler : handlers)
        handler→setFeedbackSignals(signals);
}
```

## Преимущества

- **Модульность** — каждый тип контрола в своём классе
- **Расширяемость** — новый тип = новый наследник ControlHandler
- **Безопасность** — существующая логика Toggle/Button не меняется при добавлении нового типа
- **Тестируемость** — каждый handler можно тестировать изолированно
- **Кастомные модули** — аддоны могут подгружать свои handler-ы через LOAD_MODULE

## Миграция

1. Создать `ControlHandler` с общей логикой (`getSignalValueByName`, `sendControlSignal`, проверка модификаторов)
2. Вынести Toggle/Button в `ToggleHandler` (логика из `processTumbler`, `processButton`, `mouseProcessTumbler`, `mouseProcessButton`)
3. `SwitcherHandler` уже наследует `ControlHandler`
4. IOController оперирует списком handler-ов