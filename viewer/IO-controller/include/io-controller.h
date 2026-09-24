#ifndef     IO_CONTROLLER_H
#define     IO_CONTROLLER_H

#include    <io-controller-export.h>
#include    <io-controller-input.h>
#include    <QObject>
#include    <QMap>
#include    <set>
#include    <vector>
#include    <optional>
#include    <string>
#include    <functional>

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

    void create_animations_map(const QStringList &anim_dirs);

    void setCabineIndex(int vehicle_idx, int cab_idx);

    void setVehicleSignals(const std::vector<float> *vehicle_signals);

    float getVehicleSignal(int signal_id) const;

    std::optional<io_control_input_t> getInputByObject(const QString &object_name) const;

    virtual QString getControlStateText(const io_control_input_t &io_ctrl,
                                        float state) const;

    bool findControl(const std::string &node_name, io_control_input_t &out) const;

    void mouseClick(const QString &object_name, int button);

    void mouseRelease(const QString &object_name);

    void mouseInputProcess(io_control_input_t input, uint32_t button, bool is_pressed);

    void setFeedbackSignals(const std::vector<float> *server_signals)
    {
        feedback_signals = server_signals;
    }

    float getSignalValueByName(const QString& objectName) const;

    float getSignalValueByID(uint16_t control_id, int cab_idx) const;

signals:

    void sigSendVehicleControlCommand(const QByteArray &data);

protected:

    std::set<uint16_t> _pressed_keys;

    QMap<QString, std::function<bool(const std::set<uint16_t> &)>> isModifier;

    DualKeyHash<uint16_t, QString, io_control_input_t> io_control_inputs;

    const std::vector<float> *vehicle_signals = nullptr;

    enum ControlType
    {
        CTRL_TYPE_KEYBOARD,
        CTRL_TYPE_MOUSE,
        CTRL_TYPE_CTRL_PANEL
    };

    virtual void keysProcess(std::set<uint16_t> &pressed_keys) = 0;

    void processSwitchBySignal(io_control_input_t &io_ctrl);

    void processTumbler(const uint16_t &control_id, const std::set<uint16_t> &pressed_keys);

    void processButton(const uint16_t &control_id, const std::set<uint16_t> &pressed_keys);

    void mouseProcessTumbler(io_control_input_t input, uint32_t button, bool is_pressed);

    void mouseProcessButton(io_control_input_t input, uint32_t button, bool is_pressed);

    virtual void processMouseInput(io_control_input_t input, uint32_t button, bool is_pressed);

    bool checkModKey(const QString &modKeyName, const std::set<uint16_t> &pressed_keys);

    virtual void processMouseControl(io_control_input_t &io_ctrl, int button);

    void emitControl(const io_control_input_t &io_ctrl);

private:

    QMap<QString, uint16_t> animation_signals_map;

    const std::vector<float>* feedback_signals = nullptr;

    void processKeyBoardInput();

    void processMouseInput();

    void processControlPanelInput();

    void processControl(const ControlType &ctrl_type);

    void getHotkeysString(const QString &keyName, io_control_input_t &ic_input);
    void getUsageString(io_control_input_t &ic_input);
};

#endif
