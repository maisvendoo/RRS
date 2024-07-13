#ifndef     SOUND_SIGNAL_H
#define     SOUND_SIGNAL_H

#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct sound_state_t
{
    bool  play;
    float volume;
    float pitch;

    sound_state_t(bool play = false, float volume = 0.999f, float pitch = 1.0f)
        : play(play)
        , volume(volume)
        , pitch(pitch)
    {

    }

    static float createSoundSignal(bool play)
    {
        if (play)
            return 100.999f;
        else
            return -100.999f;
    }

    static float createSoundSignal(bool play, float volume = 0.999f, float pitch = 1.0f)
    {
        float v = std::fmax( 0.0f, std::fmin(0.999f, volume) );
        float p = std::fmax( 0.4f, std::fmin(2.0f, pitch) );
        if (play)
            return floorf(100.0f * p) + v;
        else
            return -(floorf(100.0f * p) + v);
    }

    float createSoundSignal()
    {
        return createSoundSignal(play, volume, pitch);
    }

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
