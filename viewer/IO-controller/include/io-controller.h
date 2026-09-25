#ifndef     IO_CONTROLLER_H
#define     IO_CONTROLLER_H

#include    <io-controller-export.h>
#include    <io-controller-input.h>
#include    <control-handler.h>
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

    void create_animations_map(const QStringList &anim_dirs);

    void setVehicleIndex(int vehicle_idx);

    void setActirveCabineIndex(int cab_idx)
    {
        cabine_idx = cab_idx;
    }

    bool findControl(const std::string &node_name, io_control_input_t &out) const;

    /// Обработка мышиного ввода
    void mouseInputProcess(io_control_input_t input, uint32_t button, bool is_pressed);

    /// Задать массив сигналов анимаций
    void setFeedbackSignals(const std::vector<float> *server_signals);    

signals:

    void sigSendVehicleControlCommand(const QByteArray &data);

protected:

    /// Массив нажатых клавиш
    std::set<uint16_t> _pressed_keys;    

    /// Здесь обеспечивается доступ к значению сигнала контрола
    /// как по коду нажатой кавиши, так и по имени объекта, кликнутого мышью
    std::vector<DualKeyHash<uint16_t, QString, io_control_input_t>> io_control_inputs;

    int cabs_num = 0;

    int cabine_idx = 0;

    int vehicle_idx = 0;

    /// Обработка управления с клавиатуры в кастомных модулях
    virtual void processKeyboardInput(std::set<uint16_t> &pressed_keys);

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

    QMap<uint16_t, ControlHandler*> handlers;

    void create_handlers();

private:

    /// Маппинг: имя 3D-объекта → ID сигнала обратной связи из analogSignal
    QMap<QString, uint16_t> animation_signals_map;

    /// Обработка клавиатурного управления (Общая для всех часть)
    void processKeyBoardInput();

    void keyboardInputProcess(std::set<uint16_t> &pressed_keys);

    /// Сформировать строку с посказкой горячей клавиши
    void getHotkeysString(const QString &keyName, io_control_input_t &ic_input);
    void getUsageString(io_control_input_t &ic_input);
};

#endif
