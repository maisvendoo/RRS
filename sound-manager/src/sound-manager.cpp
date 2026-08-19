#include "CfgReader.h"
#include "filesystem.h"
#include "sound-manager.h"

#include <algorithm>
#include <chrono>
#include <sstream>

//------------------------------------------------------------------------------
// Число типов физических звуковых событий (SoundEventType)
//------------------------------------------------------------------------------
static constexpr size_t SOUND_EVENT_TYPES_COUNT = 10;

//------------------------------------------------------------------------------
// Имена секций конфигурации sound-events.conf по типу события
//------------------------------------------------------------------------------
static const char* sound_event_type_names[SOUND_EVENT_TYPES_COUNT] =
{
    "FlatImpact",       // Удар ползуна о рельс
    "FlangeContact",    // Скрежет гребня
    "CouplerImpact",    // Удар в сцепке
    "CouplerBreak",     // Разрыв сцепки
    "DerailmentScrape", // Скрежет сошедшей ПЕ
    "PantographArc",    // Дуга токоприёмника
    "SandFlow",         // Поток песка
    "WheelSlip",        // Боксование/юз
    "JointImpact",      // Стук на стыке
    "BrakeSqueal"       // Свист колодок
};

//------------------------------------------------------------------------------
// Файлы по умолчанию: ближайшие по смыслу из поставки data/sounds.
// Ключ File в конфиге позволяет заменить на специализированный
//------------------------------------------------------------------------------
static const char* sound_event_default_files[SOUND_EVENT_TYPES_COUNT] =
{
    "vl60/ezda.wav",          // FlatImpact - стук хода
    "vl60/brake_scr.wav",     // FlangeContact - скрежет металла
    "vl60/254-chelk.wav",     // CouplerImpact - щелчок удара
    "vl60/254_vypusk.wav",    // CouplerBreak - резкий выпуск воздуха
    "freight/departure.wav",  // DerailmentScrape - низкий гул волочения
    "vl60/gvon.wav",          // PantographArc - включение ГВ (треск)
    "vl60/compr.wav",         // SandFlow - шипение
    "vl60/brake_scr.wav",     // WheelSlip - визг
    "vl60/254-chelk.wav",     // JointImpact - стук стыка
    "vl60/brake_scr.wav"      // BrakeSqueal - свист
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SoundManager::SoundManager(QObject* parent) : QObject(parent)
{
    init();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SoundManager::~SoundManager()
{
    alcDestroyContext(context_);
    alcCloseDevice(device_);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::init()
{
    // Инициализируем лог-файл
    FileSystem &fs = FileSystem::getInstance();
    log_ = new LogFileHandler(fs.getLogsDir() + fs.separator(), "asound.log");
    log_->notify("========================== SoundManager initialization ==========================");

    // Загрузка конфиг-файла
    double tmp_volume = 1.0;
    int tmp_max_sources = 65535;

    QString cfg_path = QString(fs.getConfigDir().c_str()) + fs.separator() + "sound-settings.xml";
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Settings";

        cfg.getDouble(secName, "Volume", tmp_volume);
        cfg.getInt(secName, "MaxSources", tmp_max_sources);
    }

    ALfloat volume = std::clamp(tmp_volume, 0.0, 1.0);
    ALCint max_sources = std::clamp(tmp_max_sources, 1, 1000000);

    // Открываем устройство
    device_ = alcOpenDevice(nullptr);

    // Создаём контекст, задаём максимальное количество источников звука
    ALCint context_atrribute_list[2] = {ALC_MONO_SOURCES, max_sources};
    context_ = alcCreateContext(device_, context_atrribute_list);

    // Устанавливаем текущий контекст
    alcMakeContextCurrent(context_);

    // Устанавливаем положение слушателя в начале координат
    ALfloat pos[3] = {0.0f, 0.0f, 0.0f};
    alListenerfv(AL_POSITION, pos);

    // Устанавливаем нулевой вектор скорости слушателя
    ALfloat vel[3] = {0.0f, 0.0f, 0.0f};
    alListenerfv(AL_VELOCITY, vel);

    // Устанавливаем направление слушателя вперёд по Oy, вверх по Oz
    ALfloat ori[6] = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    alListenerfv(AL_ORIENTATION, ori);

    // Устанавливаем общий уровень громкости
    alListenerf(AL_GAIN, volume);

    log_->notify(QString("Volume: %1").arg(volume, 5, 'f', 3).toStdString());
    log_->notify(QString("Sources: %1").arg(max_sources).toStdString());
    log_->notify("=========================== Initialization successful ===========================");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<size_t> SoundManager::loadVehicleSounds(const std::string& sounddir)
{
    FileSystem &fs = FileSystem::getInstance();
    std::string dirPath = fs.getSoundsDir() + fs.separator() + sounddir;
    std::string cfgPath = dirPath + fs.separator() + "sounds.xml";

    log_->notify("Sound Manager: Start loading sounds from " + dirPath + " directory");

    return loadSounds(dirPath, cfgPath);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<size_t> SoundManager::loadSounds(const std::string& dir_path, const std::string& cfg_path)
{
    std::vector<std::size_t> sounds_id;

    CfgReader cfg;
    if (cfg.load(QString(cfg_path.c_str())))
    {
        QDomNode secNode = cfg.getFirstSection("Sound");

        while (!secNode.isNull())
        {
            sound_config_t sound_config;
            sound_config.prev_state = false;

            int id = -1;
            cfg.getInt(secNode, "SignalID", id);
            if (id < 0)
            {
                secNode = cfg.getNextSection();
                continue;
            }
            sound_config.signal_id = static_cast<size_t>(id);

            QString pos;
            cfg.getString(secNode, "LocalPosition", pos);
            std::istringstream ss(pos.toStdString());
            ss >> sound_config.local_pos_x >> sound_config.local_pos_y >> sound_config.local_pos_z;

            double tmp = 1.0;
            cfg.getDouble(secNode, "InitVolume", tmp);
            sound_config.init_volume = static_cast<float>(std::max(0.0, std::min(1.0, tmp)));

            tmp = 1.0;
            cfg.getDouble(secNode, "MaxVolume", tmp);
            sound_config.max_volume = static_cast<float>(std::max(0.0, std::min(1.0, tmp)));

            tmp = 1.0;
            cfg.getDouble(secNode, "InitPitch", tmp);
            sound_config.init_pitch = static_cast<float>(std::max(0.5, std::min(2.0, tmp)));

            cfg.getBool(secNode, "Loop", sound_config.loop);

            cfg.getBool(secNode, "PlayOnStart", sound_config.play_on_start);

            sound_config.sounddir = QString(dir_path.c_str());

            cfg.getString(secNode, "Filename", sound_config.filename);

            const QString sound_name = QString(dir_path.c_str()) + QDir::separator() + sound_config.filename;
            auto found_sound_it = loaded_sounds.find(sound_name.toStdString());
            if (found_sound_it != loaded_sounds.end())
            {
                sound_config.sound = new ASound(*found_sound_it->second, log_);
                sound_config.sound->setVolume(sound_config.init_volume * sound_config.max_volume);
                sound_config.sound->setPitch(sound_config.init_pitch);
                sound_config.sound->setLoop(sound_config.loop);

                if (sound_config.play_on_start)
                {
                    sound_config.sound->play();
                }

                sounds_id.push_back(sounds.size());
                sounds.push_back(sound_config);
            }
            else
            {
                sound_config.sound = new ASound(sound_name, log_);

                const QString tmp_error = sound_config.sound->getLastError();
                if (tmp_error.isEmpty())
                {
                    sound_config.sound->setVolume(sound_config.init_volume * sound_config.max_volume);
                    sound_config.sound->setPitch(sound_config.init_pitch);
                    sound_config.sound->setLoop(sound_config.loop);

                    if (sound_config.play_on_start)
                    {
                        sound_config.sound->play();
                    }

                    sounds_id.push_back(sounds.size());
                    sounds.push_back(sound_config);
                    loaded_sounds.emplace(sound_name.toStdString(), sound_config.sound);
                }
                else
                {
                    log_->notify(QString("Sound Manager: can't load sound #%1(total #%2): ")
                        .arg(sounds_id.size())
                        .arg(sounds.size()).toStdString() + tmp_error.toStdString());
                }
            }

            secNode = cfg.getNextSection();
        }
    }

    log_->notify("Sound Manager: Loaded " + QString("%1").arg(sounds_id.size()).toStdString() + " sounds from " + dir_path + " directory");
    log_->notify("=================== Total loaded sounds: "+ QString("%1 (unique:%2)").arg(sounds.size(), 5).arg(loaded_sounds.size(), 4).toStdString() + " ===================");

    return sounds_id;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t SoundManager::getSignalID(size_t idx)
{
    if (idx >= sounds.size())
    {
        return 0;
    }

    return sounds[idx].signal_id;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SoundManager::getLocalPositionX(size_t idx)
{
    if (idx >= sounds.size())
    {
        return 0.0f;
    }

    return sounds[idx].local_pos_x;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SoundManager::getLocalPositionY(size_t idx)
{
    if (idx >= sounds.size())
    {
        return 0.0f;
    }

    return sounds[idx].local_pos_y;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SoundManager::getLocalPositionZ(size_t idx)
{
    if (idx >= sounds.size())
    {
        return 0.0f;
    }

    return sounds[idx].local_pos_z;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::setListenerPosition(float x, float y, float z)
{
    ALfloat pos[3] = {x, y, z};
    alListenerfv(AL_POSITION, pos);

    // Позиция слушателя нужна пулу событий для дистанционной отсечки
    listener_x = x;
    listener_y = y;
    listener_z = z;
    //log_->notify("Sound Manager: ListenerPosition " + QString("%1 %2 %3").arg(x).arg(y).arg(z).toStdString());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::setListenerVelocity(float x, float y, float z)
{
    ALfloat vel[3] = {x, y, z};
    alListenerfv(AL_VELOCITY, vel);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::setListenerOrientation(float at_x, float at_y, float at_z, float up_x, float up_y, float up_z)
{
    ALfloat ori[6] ={at_x, at_y, at_z, up_x, up_y, up_z};
    alListenerfv(AL_ORIENTATION, ori);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::setPosition(size_t idx, float x, float y, float z)
{
    if (idx >= sounds.size())
    {
        return;
    }

    sounds[idx].sound->setPosition(x, y, z);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::setVelocity(size_t idx, float x, float y, float z)
{
    if (idx >= sounds.size())
    {
        return;
    }

    sounds[idx].sound->setVelocity(x, y, z);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::setSoundSignal(size_t idx, float signal)
{
    sound_state_t ss;
    setSoundState(idx, ss.soundFromSignal(signal));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::setSoundState(size_t idx, sound_state_t ss)
{
    //log_->notify(QString("State for sound [%1]: play %2 | volume %3 | pitch %4").arg(idx).arg(static_cast<int>(ss.state)).arg(ss.volume, 5, 'f', 3).arg(ss.pitch, 5, 'f', 3).toStdString());
    if (idx >= sounds.size())
    {
        return;
    }

    if (ss.volume == 0.0f)
    {
        if (sounds[idx].prev_state > 0)
        {
            sounds[idx].sound->stop();
            sounds[idx].prev_state = 0;
        }
        sounds[idx].sound->setVolume(0.0f);
        return;
    }

    sounds[idx].sound->setVolume(ss.volume * sounds[idx].max_volume);

    if (ss.pitch < 0.5f)
    {
        if (sounds[idx].prev_state > 0)
        {
            sounds[idx].sound->stop();
            sounds[idx].prev_state = 0;
        }
        sounds[idx].sound->setPitch(0.5f);
        return;
    }

    sounds[idx].sound->setPitch(ss.pitch);

    if (ss.state == 0)
    {
        if (sounds[idx].prev_state > 0)
        {
            sounds[idx].sound->stop();
            sounds[idx].prev_state = 0;
        }
    }
    else
    {
        if (sounds[idx].prev_state != ss.state)
        {
            sounds[idx].sound->play();
            sounds[idx].prev_state = ss.state;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::play(size_t idx)
{
    if (idx >= sounds.size())
    {
        return;
    }

    if (sounds[idx].prev_state == 0)
    {
        sounds[idx].sound->play();
        sounds[idx].prev_state = 1;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::stop(size_t idx)
{
    if (idx >= sounds.size())
    {
        return;
    }

    if (sounds[idx].prev_state > 0)
    {
        sounds[idx].sound->stop();
        sounds[idx].prev_state = 0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::setVolume(size_t idx, float volume)
{
    if (idx >= sounds.size())
    {
        return;
    }

    if (volume <= 0.0f)
    {
        sounds[idx].sound->setVolume(0.0f);

        if (sounds[idx].prev_state > 0)
        {
            sounds[idx].sound->stop();
            sounds[idx].prev_state = 0;
        }
        return;
    }

    if (volume >= 1.0f)
    {
        sounds[idx].sound->setVolume(sounds[idx].max_volume);
    }
    else
    {
        sounds[idx].sound->setVolume(volume * sounds[idx].max_volume);
    }

    if (sounds[idx].prev_state == 0)
    {
        sounds[idx].sound->play();
        sounds[idx].prev_state = 1;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::setPitch(size_t idx, float pitch)
{
    if (idx >= sounds.size())
    {
        return;
    }

    if (pitch < 0.5f)
    {
        if (sounds[idx].prev_state > 0)
        {
            sounds[idx].sound->stop();
            sounds[idx].prev_state = 0;
        }
        return;
    }

    sounds[idx].sound->setPitch(pitch);

    if (sounds[idx].prev_state == 0)
    {
        sounds[idx].sound->play();
        sounds[idx].prev_state = 1;
    }
}

//------------------------------------------------------------------------------
// Монотонное время, с
//------------------------------------------------------------------------------
double SoundManager::monotonicTime() const
{
    return std::chrono::duration<double>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
}

//------------------------------------------------------------------------------
// Загрузка пула физических звуковых событий: data/sounds/sound-events.conf.
// Секция на тип события: File/MinVolume/MaxDistance/Cooldown,
// секция Pool: MaxSources. Отсутствие конфига/файла - не ошибка:
// событие тихо пропускается, warning в журнал выдаётся однократно
//------------------------------------------------------------------------------
void SoundManager::initEventSounds()
{
    event_pool_loaded = true;
    event_sounds.resize(SOUND_EVENT_TYPES_COUNT);

    FileSystem &fs = FileSystem::getInstance();
    const std::string sounds_dir = fs.getSoundsDir();
    const std::string cfg_path = sounds_dir + fs.separator() + "sound-events.conf";

    CfgReader cfg;

    if (cfg.load(QString(cfg_path.c_str())))
    {
        int tmp_int = static_cast<int>(event_pool_max_sources);
        cfg.getInt("Pool", "MaxSources", tmp_int);
        event_pool_max_sources = std::clamp(tmp_int, 1, 64);

        for (size_t i = 0; i < SOUND_EVENT_TYPES_COUNT; ++i)
        {
            EventSound& event_sound = event_sounds[i];
            const QString sec_name = sound_event_type_names[i];

            QString file = "";
            if (cfg.getString(sec_name, "File", file) && !file.isEmpty())
            {
                event_sound.filename = file.toStdString();
            }
            else
            {
                event_sound.filename = sound_event_default_files[i];
            }

            double tmp = 0.0;
            if (cfg.getDouble(sec_name, "MinVolume", tmp))
            {
                event_sound.min_volume = static_cast<float>(std::clamp(tmp, 0.0, 1.0));
            }

            tmp = 0.0;
            if (cfg.getDouble(sec_name, "MaxDistance", tmp) && (tmp > 0.0))
            {
                event_sound.max_distance = static_cast<float>(tmp);
            }

            tmp = 0.0;
            if (cfg.getDouble(sec_name, "Cooldown", tmp) && (tmp >= 0.0))
            {
                event_sound.cooldown = tmp;
            }
        }
    }
    else
    {
        log_->notify("Sound Manager: sound-events.conf not found, "
                     "event sounds use defaults");
    }

    // Загрузка wav-файлов пула (отсутствие файла - тихий пропуск)
    for (size_t i = 0; i < SOUND_EVENT_TYPES_COUNT; ++i)
    {
        EventSound& event_sound = event_sounds[i];

        const QString sound_name = QString(sounds_dir.c_str()) +
                QDir::separator() + QString(event_sound.filename.c_str());

        auto found_sound_it = loaded_sounds.find(sound_name.toStdString());
        if (found_sound_it != loaded_sounds.end())
        {
            event_sound.sound = new ASound(*found_sound_it->second, log_);
        }
        else
        {
            ASound* sound = new ASound(sound_name, log_);

            if (sound->getLastError().isEmpty())
            {
                event_sound.sound = sound;
                loaded_sounds.emplace(sound_name.toStdString(), sound);
            }
            else
            {
                delete sound;
                event_sound.sound = nullptr;
            }
        }

        if (event_sound.sound != nullptr)
        {
            event_sound.sound->setLoop(false);
        }
    }

    log_->notify(QString("Sound Manager: event pool initialized "
                         "(max sources: %1)").arg(
                     static_cast<int>(event_pool_max_sources)).toStdString());
}

//------------------------------------------------------------------------------
// Воспроизведение физического звукового события через пул источников
// (ТЗ "Аудиосистема"): приоритет по интенсивности и дистанции,
// отсечка дальних (MaxDistance), перезарядка (Cooldown) и
// виртуализация - тайминги живут даже без реального источника
//------------------------------------------------------------------------------
void SoundManager::playSoundEvent(unsigned event_type,
                                  float x, float y, float z,
                                  float intensity,
                                  float rate_hz)
{
    if (!event_pool_loaded)
    {
        initEventSounds();
    }

    if (event_type >= event_sounds.size())
    {
        return;
    }

    EventSound& event_sound = event_sounds[event_type];
    const double now = monotonicTime();

    // Виртуализация: время последнего пуска обновляется независимо от
    // того, будет ли реальное воспроизведение (состояние без источника)
    if ((now - event_sound.last_play_time) < event_sound.cooldown)
    {
        return;
    }

    // Дистанционная отсечка: дальние события не играют вовсе
    const float dx = x - listener_x;
    const float dy = y - listener_y;
    const float dz = z - listener_z;
    const float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (distance > event_sound.max_distance)
    {
        return;
    }

    // Порог интенсивности события (тихие события не играем)
    if (intensity < event_sound.min_volume)
    {
        return;
    }

    // Отсутствие файла - тихий пропуск с однократным warning в журнал
    if (event_sound.sound == nullptr)
    {
        if (!event_sound.missing_warned)
        {
            event_sound.missing_warned = true;
            log_->notify(QString("Sound Manager: no wav for event '%1' "
                                 "(file '%2') - skipped silently")
                         .arg(sound_event_type_names[event_type])
                         .arg(event_sound.filename.c_str()).toStdString());
        }
        event_sound.last_play_time = now;
        return;
    }

    // Приоритет события: громкое и близкое важнее тихого и дальнего
    const float distance_factor = 1.0f - (distance / event_sound.max_distance);
    const float priority = std::clamp(intensity, 0.0f, 1.0f) *
            std::clamp(distance_factor, 0.0f, 1.0f);

    // Лимит реальных источников: вытесняем самый тихий из звучащих,
    // если новый приоритет выше; иначе событие виртуально пропускаем
    if (event_active_sources >= event_pool_max_sources)
    {
        size_t weakest = event_sounds.size();
        float weakest_priority = priority;

        for (size_t i = 0; i < event_sounds.size(); ++i)
        {
            const EventSound& candidate = event_sounds[i];

            if ((candidate.sound != nullptr) &&
                candidate.sound->isPlaying() &&
                (candidate.last_playing_priority < weakest_priority))
            {
                weakest_priority = candidate.last_playing_priority;
                weakest = i;
            }
        }

        if (weakest >= event_sounds.size())
        {
            // Все звучащие важнее - тихий пропуск (виртуализация)
            event_sound.last_play_time = now;
            return;
        }

        event_sounds[weakest].sound->stop();
        event_active_sources -= (weakest == event_type) ? 0 : 1;
    }

    // Частота повтора события задаёт тональность источника
    // (удары ползуна/стыков ускоряются со скоростью)
    float pitch = 1.0f;

    if (rate_hz > 1.0f)
    {
        pitch = std::clamp(rate_hz / 5.0f, 0.5f, 2.0f);
    }

    event_sound.sound->stop();
    event_sound.sound->setPosition(x, y, z);
    event_sound.sound->setVolume(std::clamp(intensity, 0.0f, 1.0f));
    event_sound.sound->setPitch(pitch);
    event_sound.sound->play();

    event_sound.last_play_time = now;
    event_sound.last_playing_priority = priority;
    ++event_active_sources;

    // Освобождение источников, законченных предыдущим шагом
    size_t active = 0;
    for (const auto& candidate : event_sounds)
    {
        if ((candidate.sound != nullptr) && candidate.sound->isPlaying())
        {
            ++active;
        }
    }
    event_active_sources = std::min(active, event_pool_max_sources);
}
