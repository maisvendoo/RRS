#include    <sstream>
#include	"sound-manager.h"
#include    "CfgReader.h"
#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SoundManager::SoundManager(QObject *parent) : QObject(parent)
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
    log_ = new LogFileHandler("asound.log");
    log_->notify("========================== SoundManager initialization ==========================");

    // Загрузка конфиг-файла
    double tmp_volume = 1.0;
    int tmp_max_sources = 65535;

    FileSystem &fs = FileSystem::getInstance();
    QString cfg_path = QString(fs.getConfigDir().c_str()) + fs.separator() + "sound-settings.xml";
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Settings";

        cfg.getDouble(secName, "Volume", tmp_volume);
        cfg.getInt(secName, "MaxSources", tmp_max_sources);
    }
    ALfloat volume = static_cast<ALfloat>( std::max( 0.0, std::min(1.0, tmp_volume) ) );
    ALCint max_sources = static_cast<ALCint>( std::max( 1, std::min(1000000, tmp_max_sources) ) );

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
std::vector<size_t> SoundManager::loadVehicleSounds(const QString &sounddir)
{
    FileSystem &fs = FileSystem::getInstance();
    std::string dirPath = fs.getSoundsDir() + fs.separator()
                            + sounddir.toStdString();
    std::string cfgPath = dirPath + fs.separator() + "sounds.xml";

    log_->notify("Sound Manager: Start loading sounds from " + dirPath + " directory");

    return loadSounds(dirPath, cfgPath);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<size_t> SoundManager::loadSounds(const std::string &dir_path, const std::string &cfg_path)
{
    std::vector<size_t> sounds_id;

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

            sound_config.sound = new ASound(QString(dir_path.c_str()) +
                                                    QDir::separator() +
                                                    sound_config.filename
                                            , log_);

            QString tmp_error = sound_config.sound->getLastError();
            if (tmp_error.isEmpty())
            {
                sound_config.sound->setVolume(sound_config.init_volume * sound_config.max_volume);
                sound_config.sound->setPitch(sound_config.init_pitch);
                sound_config.sound->setLoop(sound_config.loop);

                if (sound_config.play_on_start)
                    sound_config.sound->play();

                sounds_id.push_back(sounds.size());
                sounds.push_back(sound_config);
            }
            else
            {
                log_->notify(QString("Sound Manager: can't load sound #%1(total #%2): ").arg(sounds_id.size()).arg(sounds.size()).toStdString() + tmp_error.toStdString());
            }

            secNode = cfg.getNextSection();
        }
    }
    log_->notify("Sound Manager: Loaded " + QString("%1").arg(sounds_id.size()).toStdString() + " sounds from " + dir_path + " directory");
    log_->notify("========================== Total loaded sounds: "+ QString("%1").arg(sounds.size(), 5).toStdString() + " ==========================");
    return sounds_id;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t SoundManager::getSignalID(size_t idx)
{
    if (idx >= sounds.size())
        return 0;

    return sounds[idx].signal_id;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SoundManager::getLocalPositionX(size_t idx)
{
    if (idx >= sounds.size())
        return 0.0f;

    return sounds[idx].local_pos_x;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SoundManager::getLocalPositionY(size_t idx)
{
    if (idx >= sounds.size())
        return 0.0f;

    return sounds[idx].local_pos_y;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SoundManager::getLocalPositionZ(size_t idx)
{
    if (idx >= sounds.size())
        return 0.0f;

    return sounds[idx].local_pos_z;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::setListenerPosition(float x, float y, float z)
{
    ALfloat pos[3] = {x, y, z};
    alListenerfv(AL_POSITION, pos);
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
        return;

    sounds[idx].sound->setPosition(x, y, z);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::setVelocity(size_t idx, float x, float y, float z)
{
    if (idx >= sounds.size())
        return;

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
        return;

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
        return;

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
        return;

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
        return;

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
        return;

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
