#ifndef     SOUND_SIGNAL_H
#define     SOUND_SIGNAL_H

#include    <cmath>

//------------------------------------------------------------------------------
// Состояние звука и создание-расшифровка float-сигнала управления звуком
// В float-сигнале: знак - логический сигнал включить-выключить
//                  целая часть - скорость воспроизведения в процентах, 50 - 200
//                  дробная часть - громкость звука в долях единицы, 0.0 - 0.999
//------------------------------------------------------------------------------
struct sound_state_t
{
    bool  play;     ///< Сигнал включить-выключить звук
    float volume;   ///< Громкость звука, 0.0 - 1.0
    float pitch;    ///< Скорость воспроизведения звука, 0.5-2.0

    sound_state_t(bool play = false, float volume = 0.999f, float pitch = 1.0f)
        : play(play)
        , volume(volume)
        , pitch(pitch)
    {

    }

    // Сигнал включить-выключить звук, с максимальной громкостью и обычной скоростью
    static float createSoundSignal(bool play)
    {
        if (play)
            return 100.999f;
        else
            return -100.999f;
    }

    // Сигнал управления звуком
    static float createSoundSignal(bool play, float volume, float pitch = 1.0f)
    {
        // Проверяем предельные значения громкости 0.0 - 0.999
        float v = std::fmax( 0.0f, std::fmin(0.999f, volume) );

        // Проверяем предельные значения скорости, 0.5 - 2.0
        // Если меньше 0.5, звук отключится сигналом скорости 49%
        float p = std::fmax( 0.49f, std::fmin(2.0f, pitch) );

        if (play)
            return floorf(100.0f * p) + v;
        else
            return -(floorf(100.0f * p) + v);
    }

    // Сигнал состояния звука
    float createSoundSignal()
    {
        return createSoundSignal(play, volume, pitch);
    }

    // Расшифровка сигнала в состояние звука
    sound_state_t soundFromSignal(const float signal)
    {
        float tmp_signal = signal;
        if (signal < 0.0f)
        {
            play = false;
            tmp_signal = -tmp_signal;
        }
        else
        {
            play = true;
        }
        float tmp_pitch = floorf(tmp_signal);
        volume = tmp_signal - tmp_pitch;
        pitch = tmp_pitch / 100.0f;
        return *this;
    }
};

#endif // SOUND_SIGNAL_H
