#ifndef SOUND_CONFIG_H
#define SOUND_CONFIG_H

#include "asound.h"

#include <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct sound_config_t
{
    ASound*  sound = nullptr;       ///< OpenAL-источник звука
    char     prev_state = 0;        ///< Состояние счётчика включений звука на предыдущем шаге
    size_t   signal_id = 0;         ///< Индекс сигнала состояния звука
    float    local_pos_x = 0.0f;    ///< Локальная координата звука по оси X
    float    local_pos_y = 0.0f;    ///< Локальная координата звука по оси Y
    float    local_pos_z = 0.0f;    ///< Локальная координата звука по оси Z
    float    init_volume = 0.0f;    ///< Громкость по умолчанию, 0.0 - 1.0
    float    max_volume = 1.0f;     ///< Множитель к максимальной  громкости, 0.0 - 1.0
    float    init_pitch = 1.0f;     ///< Скорость воспроизведения, 0.0 - 1.0
    bool     loop = false;          ///< Признак зацикливания звука
    bool     play_on_start = false; ///< Признак включения звука
    QString  sounddir = "";         ///< Подпапка в data/sounds с файлом звука
    QString  filename = "";         ///< Файл звука
};

#endif // SOUND_CONFIG_H
