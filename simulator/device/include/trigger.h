#ifndef     TRIGGER_H
#define     TRIGGER_H

#include    <QObject>

#include    "device-export.h"
#include    "sound-signal.h"

class DEVICE_EXPORT  Trigger : public QObject
{
    Q_OBJECT

public:

    Trigger(QObject *parent = Q_NULLPTR);

    ~Trigger();

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
};

#endif // TRIGGER_H
