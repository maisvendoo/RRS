#ifndef     SOUND_CONFIG_H
#define     SOUND_CONFIG_H

#include    <QString>
#include    "asound.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct sound_config_t
{
    ASound  *sound;         ///< OpenAl-источник звука
    char    prev_state;     ///< Состояние счётчика включений звука на предыдущем шаге
    size_t  signal_id;      ///< Индекс сигнала состояния звука
    float   local_pos_x;    ///< Локальная координата звука по оси X
    float   local_pos_y;    ///< Локальная координата звука по оси Y
    float   local_pos_z;    ///< Локальная координата звука по оси Z
    float   init_volume;    ///< Громкость по умолчанию, 0.0 - 1.0
    float   max_volume;     ///< Множитель к максимальной  громкости, 0.0 - 1.0
    float   init_pitch;     ///< Скорость воспроизведения, 0.0 - 1.0
    bool    loop;           ///< Признак зацикливания звука
    bool    play_on_start;  ///< Признак включения звука
    QString sounddir;       ///< Подпапка в data/sounds с файлом звука
    QString filename;       ///< Файл звука

    sound_config_t()
        : sound(Q_NULLPTR)
        , prev_state(0)
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
