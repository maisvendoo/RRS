#ifndef     TRIGGER_CONTROL_TIMEDELAY_H
#define     TRIGGER_CONTROL_TIMEDELAY_H

#include    <QObject>
#include    "trigger-control.h"

class Timer;

//------------------------------------------------------------------------------
// Триггер с управлением по сигналам от клавиатуры,
// с возможностью задать задержку срабатывания
//------------------------------------------------------------------------------
class DEVICE_EXPORT TriggerControlTimedelay final : public TriggerControl, public QObject
{
public:

    TriggerControlTimedelay(double timeout_on = 0.0,
                            double timeout_off = 0.0);

    ~TriggerControlTimedelay();

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

private:

    void setAfterDelay();
    void resetAfterDelay();

private slots:

    void slotTimeoutProcessOn();
    void slotTimeoutProcessOff();
};

#endif // TRIGGER_CONTROL_TIMEDELAY_H
