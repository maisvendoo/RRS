#ifndef     AUTOMATIC_TRAIN_STOP_H
#define     AUTOMATIC_TRAIN_STOP_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT AutoTrainStop : public BrakeDevice
{
public:

    AutoTrainStop(QObject* parent = nullptr);

    virtual ~AutoTrainStop() = default;

    /// Задать управляющую клавишу ключа автостопа
    void setKeySymbol(std::uint16_t key_symbol);

    /// Разрешить установить ключ (для реализации одного ключа на несколько кабин)
    void allowKey(bool allow);

    /// Разрешение установить ключ (для реализации одного ключа на несколько кабин)
    bool isKeyAllowed() const;

    /// Вставить/извлечь ключ
    void insertKey(bool insert);

    /// Признак установленного ключа
    bool isKey() const;

    /// Переключить ключ: false - отключить автостоп, true - включить автостоп;
    void setKeyOn(bool state);

    /// Состояние ключа: false - автостоп отключен, true - автостоп включен
    bool isKeyOn() const;

    /// Задать подачу электропитания на удерживающую катушку
    void setPowered(bool powered);

    /// Наличие электропитания на удерживающей катушке
    bool isPowered() const;

    /// Задать давление от питательной магистрали
    void setFLpressure(double pressure);

    /// Поток в питательную магистраль
    double getFLflow() const;

    /// Задать давление от тормозной магистрали
    void setBPpressure(double pressure);

    /// Поток в тормозную магистраль
    double getBPflow() const;

    /// Задать поток в камеру над срывным клапаном (для реализации КОН)
    void setFlowAboveFailureValve(double flow);

    /// Давление в камере над срывным клапаном (для реализации КОН)
    virtual double getPressureAboveFailureValve() const;

    /// Предупреждающий свисток
    bool isWhistle() const;

    /// Автостопное экстренное торможение
    bool getEmergencyBrakeContact() const;

    enum {
        AUTOSTOP_WHISTLE = 0,                           ///< Звук свистка автостопа
        BP_DRAIN_FLOW_SOUND = 1,                        ///< // TODO // Звук опорожнения тормозной магистрали
        KEY_CHANGED = 2 + Trigger::CHANGE_SOUND,        ///< Звук установки/извлечения ключа
        KEY_INSERTED = 2 + Trigger::ON_SOUND,           ///< Звук установки ключа
        KEY_REMOVED = 2 + Trigger::OFF_SOUND,           ///< Звук извлечения ключа
        KEY_STATE_CHANGED = 5 + Trigger::CHANGE_SOUND,  ///< Звук поворота ключа
        KEY_STATE_ON = 5 + Trigger::ON_SOUND,           ///< Звук поворота во включенное положение
        KEY_STATE_OFF = 5 + Trigger::OFF_SOUND          ///< Звук поворота в выключенное положение
    };
    /// Состояние звуков автостопа
    virtual sound_state_t getSoundState(size_t idx = 0) const override;

    /// Сигнал состояния звуков автостопа
    virtual float getSoundSignal(size_t idx = 0) const override;

protected:

    /// Управляющая клавиша ключа
    std::uint16_t key_symbol = KEY_Undefined;

    /// Предыдущее состояние управляющей клавиши
    bool prev_key = false;

    /// Разрешение установить ключ (для реализации одного ключа на несколько кабин)
    bool is_key_allowed = true;

    /// Наличие электропитания
    bool is_powered = false;

    /// Признак работы предупреждающего свистка
    bool is_whistle = false;

    /// Автостопное экстренное торможение
    bool is_emergency_brake = false;

    /// Давление питательной магистрали
    double pFL = 0.0;
    /// Давление тормозной магистрали
    double pBP = 0.0;

    /// Поток в питательную магистраль
    double QFL = 0.0;
    /// Поток в тормозную магистраль
    double QBP = 0.0;

    /// Поток в камеру над срывным клапаном
    double Qabove_failure_valve = 0.0;

    /// Признак установленного ключа
    Trigger is_key = Trigger();

    /// Состояние ключа: false - отключено, true - включено
    Trigger key_state = Trigger();

    virtual void stepKeysControl(double t, double dt) override;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using GetAutoTrainStop = AutoTrainStop*(*)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_AUTO_TRAIN_STOP(ClassName) \
    extern "C" AutoTrainStop* getAutoTrainStop() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" DEVICE_EXPORT AutoTrainStop* loadAutoTrainStop(QString lib_path);

#endif // AUTOMATIC_TRAIN_STOP_H
