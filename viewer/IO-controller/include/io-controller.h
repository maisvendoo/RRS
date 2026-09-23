#ifndef     IO_CONTROLLER_H
#define     IO_CONTROLLER_H

#include    <io-controller-export.h>
#include    <io-controller-input.h>
#include    <QObject>
#include    <set>

#include    <dual-key-hash.h>
#include    <QMap>

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

    void setVehicleIndex(int vehicle_idx);

    void setActirveCabineIndex(int cab_idx)
    {
        cabine_idx = cab_idx;
    }

    bool findControl(const std::string &node_name, io_control_input_t &out) const;

    /// Обработка мышиного ввода
    void mouseInputProcess(io_control_input_t input, uint32_t button, bool is_pressed);

signals:

    void sigSendVehicleControlCommand(const QByteArray &data);

protected:

    /// Массив нажатых клавиш
    std::set<uint16_t> _pressed_keys;

    /// Массив лямбд для вызова идентификаторов
    QMap<QString, std::function<bool(const std::set<uint16_t> &)>> isModifier;

    /// Здесь обеспечивается доступ к значению сигнала контрола
    /// как по коду нажатой кавиши, так и по имени объекта, кликнутого мышью
    std::vector<DualKeyHash<uint16_t, QString, io_control_input_t>> io_control_inputs;

    int cabs_num = 0;

    int cabine_idx = 0;

    /// Обработка управления с клавиатуры в кастомных модулях
    virtual void keysProcess(std::set<uint16_t> &pressed_keys);

    /// Обработка управления мышью в кастомных модулях
    virtual void processMouseInput(io_control_input_t input, uint32_t button, bool is_pressed);

    /// Обработка контрола типа "тумблер" (с фиксацией)
    void processTumbler(size_t cab_idx, const uint16_t &control_id, const std::set<uint16_t> &pressed_keys);

    /// Обработка контрола типа "кнопка" (без фиксации)
    void processButton(size_t cab_idx, const uint16_t &control_id, const std::set<uint16_t> &pressed_keys);

    /// Обработка мышки на контроле типа "тумблер"
    void mouseProcessTumbler(io_control_input_t input, uint32_t button, bool is_pressed);

    /// Обработка мышки на контроле типа "кнопка"
    void mouseProcessButton(io_control_input_t input, uint32_t button, bool is_pressed);

private:

    std::map<uint16_t, bool> prev_key_state;
    std::map<uint16_t, bool> prev_on_active_map;

    /// Проверка модификатора
    bool checkModKey(const QString &modKeyName, const std::set<uint16_t> &pressed_keys);

    /// Обработка клавиатурного управления (Общая для всех часть)
    void processKeyBoardInput();

    /// Сформировать строку с посказкой горячей клавиши
    void getHotkeysString(const QString &keyName, io_control_input_t &ic_input);
    void getUsageString(io_control_input_t &ic_input);
};

#endif
