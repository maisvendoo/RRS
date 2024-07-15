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

    enum {
        NUM_SOUNDS = 3,
        CHANGE_SOUND = 0,   ///< Звук переключения
        ON_SOUND = 1,       ///< Звук включения
        OFF_SOUND = 2       ///< Звук выключения
    };
    /// Sound state
    virtual sound_state_t getSoundState(size_t idx = CHANGE_SOUND) const;

    /// Sound state (as a single float value, see common-headers/sound-signal.h)
    virtual float getSoundSignal(size_t idx = CHANGE_SOUND) const;

private:

    /// Состояние триггера
    bool state;

    /// Не создаём звук выключения при инициализации
    bool was_first_reset;

    /// Звук переключения (со счётчиком включений звука)
    sound_state_t sound_change_state;
};

#endif // TRIGGER_H
