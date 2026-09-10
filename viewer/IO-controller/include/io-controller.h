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

signals:

    void sigSendVehicleControlCommand(const QByteArray &data);

protected:

    /// Массив нажатых клавиш
    std::set<uint16_t> _pressed_keys;

    /// Здесь обеспечивается доступ к значению сигнала контрола
    /// как по коду нажатой кавиши, так и по имени объекта, кликнутого мышью
    DualKeyHash<uint16_t, QString, io_control_input_t> io_control_inputs;

    enum ControlType
    {
        CTRL_TYPE_KEYBOARD,
        CTRL_TYPE_MOUSE,
        CTRL_TYPE_CTRL_PANEL
    };

    virtual void keysProcess(std::set<uint16_t> &pressed_keys);

    // Обработка контрола типа "тумблер" (с фиксацией)
    void processTumbler(const uint16_t &control_id, const std::set<uint16_t> &pressed_keys);

private:

    /// Обработка клавиатурного управления (Общая для всех часть)
    void processKeyBoardInput();

    /// Обработка управления мышью (Общая для всех часть)
    void processMouseInput();

    /// Обработка управления с пульта тренажера (Общая для всех часть)
    void processControlPanelInput();

    /// Обработка управления
    void processControl(const ControlType &ctrl_type);
};

#endif
