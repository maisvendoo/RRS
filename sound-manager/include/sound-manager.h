#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include "asound-log.h"
#include "sound-config.h"
#include "sound-export.h"
#include "sound-signal.h"

#include <QObject>

#include <cstddef>
#include <chrono>
#include <map>
#include <string>
#include <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SOUND_MANAGER_EXPORT SoundManager : public QObject
{
    Q_OBJECT

public:
    SoundManager(QObject* parent = nullptr);
    ~SoundManager();

    /// Загрузка звуков для единицы подвижного состава.
    /// Возвращает список порядковых номеров загруженных звуков
    std::vector<std::size_t> loadVehicleSounds(const std::string& sound_dir);

    /// Номер управляющего сигнала от единицы подвижного состава для звука #idx
    std::size_t getSignalID(std::size_t idx);

    /// Локальные координаты звука #idx относительно единицы подвижного состава
    float getLocalPositionX(std::size_t idx);
    float getLocalPositionY(std::size_t idx);
    float getLocalPositionZ(std::size_t idx);

    /// Воспроизвести физическое звуковое событие (ТЗ "Аудиосистема"):
    /// пул источников с приоритетом по интенсивности/дистанции,
    /// отсечкой дальних и виртуализацией состояния без источника.
    /// event_type - значение SoundEventType (0..9)
    void playSoundEvent(unsigned event_type,
                        float x, float y, float z,
                        float intensity,
                        float rate_hz);

    /// Лог-файл
    LogFileHandler* log_ = nullptr;

private:
    /// Аудиоустройство
    ALCdevice* device_ = nullptr;

    /// Контекст OpenAL
    ALCcontext* context_ = nullptr;

    /// Массив звуков OpenAL и их параметров
    std::vector<sound_config_t> sounds;

    /// Уже загруженные звуки
    std::map<std::string, ASound*> loaded_sounds;

    /// Параметры одного типа физических звуковых событий
    struct EventSound
    {
        ASound* sound = nullptr;            ///< Источник (nullptr - файла нет)
        std::string filename = "";          ///< Файл из data/sounds
        float min_volume = 0.05f;           ///< Порог интенсивности события
        float max_distance = 500.0f;        ///< Дистанционная отсечка, м
        double cooldown = 0.05;             ///< Минимальный пауза между пусками, с
        double last_play_time = -1.0e9;     ///< Время последнего пуска (виртуально)
        double last_playing_priority = 0.0; ///< Приоритет последнего пуска
        bool missing_warned = false;        ///< Warning об отсутствии файла выдан
    };

    /// Пул физических звуковых событий, индекс = тип (SoundEventType)
    std::vector<EventSound> event_sounds;

    /// Лимит одновременно звучащих событий (реальных источников), шт
    size_t event_pool_max_sources = 8;

    /// Число активных источников пула
    size_t event_active_sources = 0;

    /// Пул инициализирован (конфиг sound-events.conf прочитан)
    bool event_pool_loaded = false;

    /// Позиция слушателя (для дистанционной отсечки), м
    float listener_x = 0.0f;
    float listener_y = 0.0f;
    float listener_z = 0.0f;

    /// Монотонное время, с
    double monotonicTime() const;

    /// Загрузка конфигурации пула событий (sound-events.conf)
    void initEventSounds();

    /// Инициализация
    void init();

    /// Загрузка звуков, возвращает список порядковых номеров загруженных звуков
    std::vector<size_t> loadSounds(const std::string &dir_path, const std::string &cfg_path);

public slots:

    // Слушатель
    /// Задать положение слушателя
    void setListenerPosition(float x, float y, float z);

    /// Задать вектор скорости слушателя
    void setListenerVelocity(float x, float y, float z);

    /// Задать векторы вперёд и вверх ориентации слушателя в пространстве
    void setListenerOrientation(float at_x, float at_y, float at_z, float up_x = 0.0f, float up_y = 0.0f, float up_z = 1.0f);

    // Источник звука
    /// Задать положение источника звука #idx
    void setPosition(size_t idx, float x, float y, float z);

    /// Задать вектор скорости источника звука #idx
    void setVelocity(size_t idx, float x, float y, float z);

    /// Задать сигнал состояния (счётчик включений, громкость, скорость воспроизведения) для источника звука #idx
    void setSoundSignal(size_t idx, float signal);

    /// Задать состояние (счётчик включений, громкость, скорость воспроизведения) для источника звука #idx
    void setSoundState(size_t idx, sound_state_t ss);

    /// Включить источник звука #idx
    void play(size_t idx);

    /// Выключить источник звука #idx
    void stop(size_t idx);

    /// Задать громкость источнику звука #idx
    void setVolume(size_t idx, float volume);

    /// Задать скорость воспроизведения источнику звука #idx
    void setPitch(size_t idx, float pitch);
};

#endif // SOUND_MANAGER_H
