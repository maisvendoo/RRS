#ifndef     KEY_TRIGGER_H
#define     KEY_TRIGGER_H

#include    "device.h"
#include    "trigger.h"

class Timer;

class DEVICE_EXPORT KeyTrigger : public Device
{
public:

    KeyTrigger(int key_code = 0, double timeout_on = 0.0, double timeout_off = 0.0, QObject *parent = nullptr);

    ~KeyTrigger();

    /// Задать управляющую клавишу
    void setKeyCode(int key_code);

    /// Задать время поддержания сигнала для включения, с
    void setTimeoutOn(double timeout);

    /// Задать время поддержания сигнала для отключения, с
    void setTimeoutOff(double timeout);

    /// Задать состояние, игнорируя таймеры
    void setInitState(bool state);

    void set();

    void reset();

    bool getState() const;

    /// Целевое состояние (полезно для анимаций кнопок)
    bool getRefState() const;

    void step(double t, double dt) override;

    /// Sound state
    virtual sound_state_t getSoundState(size_t idx = Trigger::CHANGE_SOUND) const override;

    /// Sound state (as a single float value, see common-headers/sound-signal.h)
    virtual float getSoundSignal(size_t idx = Trigger::CHANGE_SOUND) const override;

protected:

    /// Управляющая клавиша
    int keyCode = 0;

    Trigger trigger;
    Timer   *timer_on = nullptr;
    Timer   *timer_off = nullptr;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

    void stepKeysControl(double t, double dt) override;

private slots:

    void slotTimeoutProcessOn();
    void slotTimeoutProcessOff();
};

#endif // KEY_TRIGGER_H
