#ifndef		SOUND_MANAGER_H
#define		SOUND_MANAGER_H

#include    "sound-export.h"

#include    <QObject>
#include    <vector>

#include    "sound-signal.h"
#include    "sound-config.h"
#include    "asound-log.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SOUND_MANAGER_EXPORT SoundManager : public QObject
{
    Q_OBJECT

public:

    SoundManager(QObject *parent = Q_NULLPTR);

    ~SoundManager();

    /// Загрузка звуков для единицы подвижного состава.
    /// Возвращает список порядковых номеров загруженных звуков
    std::vector<size_t> loadVehicleSounds(const QString &sounddir);

    /// Номер управляющего сигнала от единицы подвижного состава для звука #idx
    size_t getSignalID(size_t idx);

    /// Локальные координаты звука #idx относительно единицы подвижного состава
    float getLocalPositionX(size_t idx);
    float getLocalPositionY(size_t idx);
    float getLocalPositionZ(size_t idx);

    /// Лог-файл
    LogFileHandler *log_;

private:

    /// Аудиоустройство
    ALCdevice* device_;

    /// Контекст OpenAL
    ALCcontext* context_;

    /// Массив звуков OpenAL и их параметров
    std::vector<sound_config_t> sounds;

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
