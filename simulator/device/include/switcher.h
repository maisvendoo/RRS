#ifndef SWITCHER_H
#define SWITCHER_H

#include    <QObject>
#include    <cstdint>

#include    "device-export.h"
#include    "sound-signal.h"

//------------------------------------------------------------------------------
// Многопозиционный переключатель
//------------------------------------------------------------------------------
class DEVICE_EXPORT Switcher
{
public:

    Switcher(std::uint16_t num_positions = 2);

    ~Switcher() = default;

    /// Задать количество позиций
    void setNumPositions(std::uint16_t num_positions);

    /// Задать начальную позицию (без озвучки)
    void setInitPosition(std::uint16_t position);

    /// Задать позицию
    void setPosition(std::uint16_t position);

    /// Следующая позиция
    void incPos();

    /// Предыдущая позиция
    void decPos();

    /// Количество позиций
    std::uint16_t getNumPositions() const;

    /// Текущая позиция
    std::uint16_t getPosition() const;

    /// Текущее относительное положение переключателя, 0.0 - 1.0
    float getHandlePosition() const;

    /// Состояние позиции
    bool isSwitched(std::uint16_t pos) const;

    /// Звук переключения позиции
    virtual sound_state_t getSoundState(size_t idx = 0) const;

    /// Сигнал звука переключения позиции
    virtual float getSoundSignal(size_t idx = 0) const;

protected:

    /// Текущая позиция
    std::uint16_t state = 0;

    /// Количество позиций переключателя
    std::uint16_t num_states = 2;

    /// Звук переключения позиции
    sound_state_t switch_sound = sound_state_t();
};

#endif // SWITCHER_H
