#ifndef     SOUND_CONFIG_H
#define     SOUND_CONFIG_H

#include    <QString>
#include    "asound.h"

struct sound_config_t
{
    ASound      *sound;
    QString     name;
    QString     path;
    int         init_volume;
    int         max_volume;    
    float       init_pitch;
    bool        loop;
    bool        play_on_start;

    sound_config_t()
        : sound(Q_NULLPTR)
        , name("")
        , path("")
        , init_volume(0)
        , max_volume(100)
        , init_pitch(1.0)
        , loop(false)
        , play_on_start(false)
    {

    }
};

#endif // SOUND_CONFIG_H
