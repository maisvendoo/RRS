#ifndef     TRIGGER_H
#define     TRIGGER_H

#include    <QObject>

#include    "device-export.h"
#include    "sound-signal.h"
#include    "timer.h"

class DEVICE_EXPORT  Trigger : public QObject
{
    Q_OBJECT

public:

    Trigger(QObject *parent = Q_NULLPTR);

    ~Trigger();

    /// Step (excecuted in integraion step)
    void step(double t, double dt);

    void set();

    void reset();

    bool getState() const;

    /// Состояние звука включения триггера
    sound_state_t getSoundOn() const;

    /// Состояние звука выключения триггера
    sound_state_t getSoundOff() const;

private:

    bool state;

    /// Состояние звука включения
    sound_state_t sound_on;

    /// Состояние звука отключения
    sound_state_t sound_off;

    Timer *sound_on_reset;
    Timer *sound_off_reset;

private slots:

    void slotSoundOnReset();
    void slotSoundOffReset();
};

#endif // TRIGGER_H
