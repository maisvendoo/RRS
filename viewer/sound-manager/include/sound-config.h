#ifndef     SOUND_CONFIG_H
#define     SOUND_CONFIG_H

#include    <QString>
#include    "asound.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct sound_config_t
{
    ASound  *sound;
    bool    prev_play;
    size_t  signal_id;
    float   local_pos_x;
    float   local_pos_y;
    float   local_pos_z;
    float   init_volume;
    float   max_volume;
    float   init_pitch;
    bool    loop;
    bool    play_on_start;
    QString sounddir;
    QString filename;

    sound_config_t()
        : sound(Q_NULLPTR)
        , prev_play(false)
        , signal_id(0)
        , local_pos_x(0.0f)
        , local_pos_y(0.0f)
        , local_pos_z(0.0f)
        , init_volume(0.0f)
        , max_volume(1.0f)
        , init_pitch(1.0f)
        , loop(false)
        , play_on_start(false)
        , sounddir("")
        , filename("")
    {

    }
};

#endif // SOUND_CONFIG_H
