#ifndef     IO_CONTROLLER_H
#define     IO_CONTROLLER_H

#include    <io-controller-export.h>
#include    <io-controller-input.h>
#include    <QObject>
#include    <set>

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

    bool findControl(const std::string &node_name, io_control_input_t &out) const;

    /// Обработка мышиного ввода
    void mouseInputProcess(io_control_input_t input, uint32_t button, bool is_pressed);

signals:

    void sigSendVehicleControlCommand(const QByteArray &data);

protected:

    /// Массив нажатых клавиш
    std::set<uint16_t> _pressed_keys;

    /// Здесь обеспечивается доступ к значению сигнала контрола
    /// как по коду нажатой кавиши, так и по имени объекта, кликнутого мышью
    DualKeyHash<uint16_t, QString, io_control_input_t> io_control_inputs;    

    /// Обработка управления с клавиатуры в кастомных модулях
    virtual void keysProcess(std::set<uint16_t> &pressed_keys);

    /// Обработка управления мышью в кастомных модулях
    virtual void processMouseInput(io_control_input_t input, uint32_t button, bool is_pressed);

    /// Обработка контрола типа "тумблер" (с фиксацией)
    void processTumbler(const uint16_t &control_id, const std::set<uint16_t> &pressed_keys);

    /// Обработка контрола типа "кнопка" (без фиксации)
    void processButton(const uint16_t &control_id, const std::set<uint16_t> &pressed_keys);

    /// Обработка мышки на контроле типа "тумблер"
    void mouseProcessTumbler(io_control_input_t input, uint32_t button, bool is_pressed);

    /// Обработка мышки на контроле типа "кнопка"
    void mouseProcessButton(io_control_input_t input, uint32_t button, bool is_pressed);

private:

    /// Проверка модификатора
    bool checkModKey(const QString &modKeyName, const std::set<uint16_t> &pressed_keys);

    /// Обработка клавиатурного управления (Общая для всех часть)
    void processKeyBoardInput();

    /// Сформировать строку с посказкой горячей клавиши
    void getHotkeysString(const QString &keyName, io_control_input_t &ic_input);
    void getUsageString(io_control_input_t &ic_input);
};

#endif
