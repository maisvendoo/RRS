#ifndef     CONTROL_HANDLER_H
#define     CONTROL_HANDLER_H

#include    <QObject>

#include    <set>

#include    <dual-key-hash.h>
#include    <io-controller-input.h>
#include    <io-controller-keymap.h>

class CfgReader;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ControlHandler : public QObject
{
    Q_OBJECT

public:

    explicit ControlHandler(QObject *parent = nullptr) : QObject(parent)
    {

    }

    virtual ~ControlHandler() = default;

    void setFeedbackSignals(const std::vector<float> *feedback_signals)
    {
        this->feedback_signals = feedback_signals;
    }

    void setAnimationSignalsMap(const QMap<QString, uint16_t> *animation_signals_map)
    {
        this->animation_signals_map = animation_signals_map;
    }

    void setControlInputs(std::vector<DualKeyHash<uint16_t, QString, io_control_input_t>>* inputs)
    {
        ctrl_inputs = inputs;
    }

    /// Загрузка конфигурации из IOControllerConfig
    virtual bool load_config(CfgReader &cfg, int cabs_num);

    /// Обработка клавиатурного ввода
    virtual void processKeyInput(const std::set<uint16_t>& pressed_keys,
                                 int cabine_idx, int vehicle_idx) = 0;

    /// Обработка мышиного ввода
    virtual void processMouseInput(const io_control_input_t& input,
                                   uint32_t button, bool is_pressed) = 0;

    virtual void step(float dt) {};

signals:

    void sigSendControlCommand(const QByteArray &data);

protected:

    /// Указатель на DualKeyHash IOController (данные и состояние)
    std::vector<DualKeyHash<uint16_t, QString, io_control_input_t>>* ctrl_inputs = nullptr;

    const std::vector<float> *feedback_signals = nullptr;

    const QMap<QString, uint16_t> *animation_signals_map = nullptr;

    /// Получить текущее значение сигнала по имени 3D-объекта
    float getSignalValueByName(const QString& objectName) const;

    /// Получить текущее значение сигнала по ID контрола
    float getSignalValueByID(uint16_t control_id, int cab_idx) const;

    //bool getKeyState(const std::set<uint16_t>& keys, uint16_t key);

    bool isKeyModifier(const std::set<uint16_t>& keys, const QString& modName);

    void sendControlSignal(const io_control_input_t &input);
};

#endif
