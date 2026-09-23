#ifndef     IO_CONTROLLER_H
#define     IO_CONTROLLER_H

#include    <io-controller-export.h>
#include    <io-controller-input.h>
#include    <QObject>
#include    <set>
#include    <vector>
#include    <optional>
#include    <string>

#include    <dual-key-hash.h>

class CfgReader;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class IO_CONTROLLER_EXPORT IOController : public QObject
{
    Q_OBJECT

public:

    IOController(QObject *parent = nullptr);

    ~IOController() = default;

    void setPressedKey(uint16_t keyBase);

    void setReleasedKey(uint16_t keyBase);

    virtual void step(float t, float dt);

    virtual bool load_config(CfgReader &cfg);

    void setCabineIndex(int vehicle_idx, int cab_idx);

    /// Сигналы управляемой ПЕ (вектор analogSignal сервера) - для
    /// чтения текущего состояния органов при кликах и подсказках
    void setVehicleSignals(const std::vector<float> *vehicle_signals);

    /// Текущее значение сигнала ПЕ по индексу, -1 если недоступен
    float getVehicleSignal(int signal_id) const;

    /// Описание органа по имени объекта 3D-модели (подсказки, пикинг)
    std::optional<io_control_input_t> getInputByObject(const QString &object_name) const;

    virtual QString getControlStateText(const io_control_input_t &io_ctrl,
                                        float state) const;

    virtual QString pickSyntheticControl(const std::string &mesh_name,
                                         float local_x,
                                         float local_y) const;

    /// Поиск органа по имени ноды меша из 3D-модели: точное совпадение
    /// с ObjectName либо суффикс (имена нод могут иметь префикс)
    bool findControl(const std::string &node_name, io_control_input_t &out) const;

    /// Клик мышью по органу: button 1 - ЛКМ, 3 - ПКМ
    void mouseClick(const QString &object_name, int button);

    /// Отпускание кнопки мыши после нажатия органу типа Button
    /// (моментальные кнопки: удержание мыши = удержание кнопки)
    void mouseRelease(const QString &object_name);

    /// Обработка мышиного ввода (точка входа апстримного MouseControlHandler:
    /// press/release семантика + кастомный хук processMouseInput)
    void mouseInputProcess(io_control_input_t input, uint32_t button, bool is_pressed);

signals:

    void sigSendVehicleControlCommand(const QByteArray &data);

protected:

    /// Массив нажатых клавиш
    std::set<uint16_t> _pressed_keys;

    /// Здесь обеспечивается доступ к значению сигнала контрола
    /// как по коду нажатой клавиши, так и по имени объекта, кликнутого мышью
    DualKeyHash<uint16_t, QString, io_control_input_t> io_control_inputs;

    /// Сигналы управляемой ПЕ (обновляются VehiclesHandler-ом)
    const std::vector<float> *vehicle_signals = nullptr;

    /// Фильтр кабины при загрузке конфига: -1 - все записи, иначе
    /// только своя кабина. Один контроллер обслуживает одну кабину,
    /// общий xml читают оба (имена мешей кабин различаются)
    int cabine_filter = -1;

    enum ControlType
    {
        CTRL_TYPE_KEYBOARD,
        CTRL_TYPE_MOUSE,
        CTRL_TYPE_CTRL_PANEL
    };

    virtual void keysProcess(std::set<uint16_t> &pressed_keys) = 0;

    /// Общая семантика органа-переключателя: клик меняет состояние на
    /// противоположное, целевое значение берётся из текущего сигнала ПЕ
    /// (сервер - источник истины). Какие органы так работают - решает
    /// модуль локомотива
    void processSwitchBySignal(io_control_input_t &io_ctrl);

    /// Обработка клавиатурного тумблера (с фиксацией):
    /// Shift+клавиша - включить, Ctrl+клавиша - выключить
    void processTumbler(const uint16_t &control_id, const std::set<uint16_t> &pressed_keys);

    /// Обработка клавиатурной кнопки (без фиксации):
    /// удержание клавиши - нажата, отпускание - отпущена
    void processButton(const uint16_t &control_id, const std::set<uint16_t> &pressed_keys);

    /// Обработка мышки на контроле типа "тумблер" (пресс-релиз семантика)
    void mouseProcessTumbler(io_control_input_t input, uint32_t button, bool is_pressed);

    /// Обработка мышки на контроле типа "кнопка" (пресс-релиз семантика)
    void mouseProcessButton(io_control_input_t input, uint32_t button, bool is_pressed);

    /// Обработка управления мышью в кастомных модулях
    virtual void processMouseInput(io_control_input_t input, uint32_t button, bool is_pressed);

    /// Проверка модификатора по имени из конфига ("Shift", "Control", "Alt")
    bool checkModKey(const QString &modKeyName, const std::set<uint16_t> &pressed_keys);

    /// Обработка клика мышью по органу (переопределяется аддоном
    /// для специфичной семантики: краны, контроллер машиниста и т.д.)
    virtual void processMouseControl(io_control_input_t &io_ctrl, int button);

    /// Отправить команду управления (обновляет кэш и эмитит сигнал)
    void emitControl(const io_control_input_t &io_ctrl);

private:

    /// Обработка клавиатурного управления (Общая для всех часть)
    void processKeyBoardInput();

    /// Обработка управления мышью (Общая для всех часть)
    void processMouseInput();

    /// Обработка управления с пульта тренажера (Общая для всех часть)
    void processControlPanelInput();

    /// Обработка управления
    void processControl(const ControlType &ctrl_type);

    /// Сформировать строку с подсказкой горячей клавиши
    void getHotkeysString(const QString &keyName, io_control_input_t &ic_input);
    void getUsageString(io_control_input_t &ic_input);
};

#endif
