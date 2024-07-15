#ifndef     SOUND_SIGNAL_H
#define     SOUND_SIGNAL_H

#include    <cmath>

//------------------------------------------------------------------------------
// Состояние звука и создание-расшифровка float-сигнала управления звуком.
// В float-сигнале: знак - логический сигнал включить-выключить;
//                  целая часть - скорость воспроизведения в процентах, 50 - 200;
//                  дробная часть - громкость звука в долях единицы, 0.0 - 0.999;
//------------------------------------------------------------------------------
struct sound_state_t
{
    char  state;  ///< Счётчик включений: выключить (0), включить (>= 0), включить снова (>= 0 и не равен предыдущему значению)
    float volume; ///< Громкость звука, 0.0 - 1.0
    float pitch;  ///< Скорость воспроизведения звука, 0.5-2.0

    sound_state_t(char play = false, float volume = 0.999f, float pitch = 1.0f)
        : state(play)
        , volume(volume)
        , pitch(pitch)
    {

    }

    // Управление выключением-включением (в т.ч. повторным) звука
    void play(bool is_play = true)
    {
        // Проверяем выключение звука
        if (!is_play)
        {
            state = 0;
            return;
        }
        // Зацикливаем счётчик включений, чтобы не переполнить
        if (state >= 3)
        {
            state = 1;
            return;
        }
        // Счётчик включений
        state++;
    }

    // Сигнал включить-выключить звук, с максимальной громкостью и обычной скоростью
    static float createSoundSignal(bool is_play)
    {
        if (is_play)
            return 100.999f;
        else
            return -100.999f;
    }

    static float createSoundSignal(char state, float volume, float pitch = 1.0f)
    {
        // Проверяем предельные значения громкости 0.0 - 0.999
        float v = std::fmax( 0.0f, std::fmin(0.999f, volume) );

        // Проверяем предельные значения скорости, 0.5 - 2.0
        // Если меньше 0.5, звук отключится сигналом скорости 49%
        float p = std::fmax( 0.49f, std::fmin(2.0f, pitch) );

        // Сигнал: скорость воспроизведения в процентах - целая часть,
        // уровень громкости - дробная часть
        float signal = floorf(100.0f * p) + v;

        // Сигнал при выключенном звуке - отрицательный
        if (state <= 0)
            return -signal;

        // Сигнал при включённом звуке (в т.ч. после обнуления счётчика включений)
        if (state == 1)
            return signal;

        // Сигнал при повторном включении звука (state >= 2)
        // Добавляем по 300 к целой части несколько раз, по счётчику включений
        float add_play_num = static_cast<float>(state - 1) * 300.0f;
        return signal + add_play_num;
    }

    // Сигнал состояния звука
    float createSoundSignal() const
    {
        return createSoundSignal(state, volume, pitch);
    }

    // Расшифровка сигнала в состояние звука
    sound_state_t soundFromSignal(const float signal)
    {
        float tmp_signal = signal;

        // Отрицательный сигнал - звук выключен
        if (signal < 0.0f)
        {
            state = 0;
            tmp_signal = -tmp_signal;
        }
        else
        {
            // Звук включен
            state = 1;
            // Проверяем счётчик включений
            while (tmp_signal >= 300.0f)
            {
                tmp_signal = tmp_signal - 300.0f;
                state++;
            }
        }

        // Расшифровка скорости воспроизведения и громкости
        float tmp_pitch = floorf(tmp_signal);
        volume = tmp_signal - tmp_pitch;
        pitch = tmp_pitch / 100.0f;
        return *this;
    }
};

#endif // SOUND_SIGNAL_H
