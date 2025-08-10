#ifndef     TRIGGER_CONTROL_TIMEDELAY_H
#define     TRIGGER_CONTROL_TIMEDELAY_H

#include    <QObject>
#include    "trigger-control.h"

class Timer;

class DEVICE_EXPORT TriggerControlTimedelay final : public TriggerControl, public QObject
{
public:

    TriggerControlTimedelay(std::uint16_t key_code = KEY_Undefined,
                            double timeout_on = 0.0,
                            double timeout_off = 0.0);

    ~TriggerControlTimedelay();

    /// Включить триггер
    void set() override;

    /// Отключить триггер
    void reset() override;

    /// Задать время поддержания сигнала для включения, с
    void setTimeoutOn(double timeout);

    /// Задать время поддержания сигнала для отключения, с
    void setTimeoutOff(double timeout);

    /// Целевое состояние (полезно для анимаций кнопок)
    bool getRefState() const;

    void step(double t, double dt) override;

protected:

    Timer* timer_on = nullptr;
    Timer* timer_off = nullptr;

private slots:

    void slotTimeoutProcessOn();
    void slotTimeoutProcessOff();
};

#endif // TRIGGER_CONTROL_TIMEDELAY_H
