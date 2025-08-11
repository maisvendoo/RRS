#ifndef     TRIGGER_H
#define     TRIGGER_H

#include    <QObject>

#include    "device-export.h"
#include    "sound-signal.h"

//------------------------------------------------------------------------------
// Триггер - логический элемент, с сигналом для озвучки смены состояния
//------------------------------------------------------------------------------
class DEVICE_EXPORT Trigger
{
public:

    Trigger() = default;

    ~Trigger() = default;

    /// Задать начальное состояние (без озвучки)
    void setInitState(bool is_state_true);

    /// Включить триггер
    void set();

    /// Отключить триггер
    void reset();

    /// Состояние триггера
    virtual bool getState() const;

    enum {
        NUM_SOUNDS = 3,
        CHANGE_SOUND = 0,   ///< Звук переключения
        ON_SOUND = 1,       ///< Звук включения
        OFF_SOUND = 2       ///< Звук выключения
    };
    /// Состояние звука
    virtual sound_state_t getSoundState(size_t idx = CHANGE_SOUND) const;

    /// Сигнал состояния звука
    virtual float getSoundSignal(size_t idx = CHANGE_SOUND) const;

protected:

    /// Состояние триггера
    bool state = false;

    // Переменные для наследника TriggerControl, поскольку всё равно выравнивание до 4х байт
    bool prev_key = false;  ///< Предыдущее состояние клавиши включения
    bool is_button = false; ///< Режим "кнопка" (не задана клавиша отключения) - триггер включен, пока зажата клавиша включения
    bool is_toogle = false; ///< Режим "переключатель" (клавиши включения и отключения совпадают) - нажатие на клавишу переключает триггер

    /// Звук переключения (со счётчиком включений звука)
    sound_state_t sound_change_state = sound_state_t();
};

#endif // TRIGGER_H
