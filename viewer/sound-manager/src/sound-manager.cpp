#include    <sstream>
#include	"sound-manager.h"
#include    "CfgReader.h"
#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SoundManager::SoundManager(QObject *parent) : QObject(parent)
{
    AListener listener = AListener::getInstance();
    connect(this, &SoundManager::notify, listener.log_, &LogFileHandler::notify);

    ALfloat pos[3] = {0.0f, 0.0f, 0.0f};
    alListenerfv(AL_POSITION, pos);
    ALfloat vel[3] = {0.0f, 0.0f, 0.0f};
    alListenerfv(AL_VELOCITY, vel);
    ALfloat ori[6] = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    alListenerfv(AL_ORIENTATION, ori);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SoundManager::~SoundManager()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<size_t> SoundManager::loadSounds(const QString &sounddir)
{
    std::vector<size_t> sounds_id;

    FileSystem &fs = FileSystem::getInstance();
    std::string soundsDir = fs.getSoundsDir() + fs.separator()
                            + sounddir.toStdString();
    std::string path = soundsDir + fs.separator() + "sounds.xml";

    CfgReader cfg;
    if (cfg.load(QString(path.c_str())))
    {
        QDomNode secNode = cfg.getFirstSection("Sound");

        while (!secNode.isNull())
        {
            sound_config_t sound_config;
            sound_config.prev_play = false;

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

            sound_config.sounddir = sounddir;

            cfg.getString(secNode, "Filename", sound_config.filename);

            sound_config.sound = new ASound(QString((soundsDir + fs.separator()).c_str())
                                            + sound_config.filename);

            if (sound_config.sound->getLastError().isEmpty())
            {
                sound_config.sound->setVolume(100.0f * sound_config.init_volume * sound_config.max_volume);
                sound_config.sound->setPitch(sound_config.init_pitch);
                sound_config.sound->setLoop(sound_config.loop);

                if (sound_config.play_on_start)
                    sound_config.sound->play();

                sounds_id.push_back(sounds.size());
                sounds.push_back(sound_config);
            }

            secNode = cfg.getNextSection();
        }
    }
    emit notify("Sound Manager: Loaded " + QString("%1").arg(sounds_id.size()).toStdString() + " sounds from data/sounds/" + sounddir.toStdString() + " directory");
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
    //emit notify("Sound Manager: ListenerPosition " + QString("%1 %2 %3").arg(x).arg(y).arg(z).toStdString());
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
void SoundManager::setSoundState(size_t idx, sound_state_t ss)
{
    emit notify(QString("State for sound [%1]: play %2 | volume %3 | pitch %4").arg(idx).arg(ss.play).arg(ss.volume, 5, 'f', 3).arg(ss.pitch, 5, 'f', 3).toStdString());

    if (idx >= sounds.size())
        return;

    if (ss.volume == 0.0f)
    {
        if (sounds[idx].prev_play)
        {
            sounds[idx].sound->stop();
            sounds[idx].prev_play = false;
        }
        sounds[idx].sound->setVolume(0);
        return;
    }
    int tmp = static_cast<int>(100.0f * ss.volume * sounds[idx].max_volume);
    sounds[idx].sound->setVolume(tmp);

    if (ss.pitch < 0.5f)
    {
        if (sounds[idx].prev_play)
        {
            sounds[idx].sound->stop();
            sounds[idx].prev_play = false;
        }
        sounds[idx].sound->setPitch(0.5f);
        return;
    }
    sounds[idx].sound->setPitch(ss.pitch);

    if (ss.play)
    {
        if (!sounds[idx].prev_play)
        {
            sounds[idx].sound->play();
            sounds[idx].prev_play = true;
        }
    }
    else
    {
        if (sounds[idx].prev_play)
        {
            sounds[idx].sound->stop();
            sounds[idx].prev_play = false;
        }
        return;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::play(size_t idx)
{
    if (idx >= sounds.size())
        return;

    if (!sounds[idx].prev_play)
    {
        sounds[idx].sound->play();
        sounds[idx].prev_play = true;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::stop(size_t idx)
{
    if (idx >= sounds.size())
        return;

    if (sounds[idx].prev_play)
    {
        sounds[idx].sound->stop();
        sounds[idx].prev_play = false;
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
        sounds[idx].sound->setVolume(0);

        if (sounds[idx].prev_play)
        {
            sounds[idx].sound->stop();
            sounds[idx].prev_play = false;
        }
        return;
    }

    if (volume >= 1.0f)
    {
        int tmp = static_cast<int>(100.0f * sounds[idx].max_volume);
        sounds[idx].sound->setVolume(tmp);
    }
    else
    {
        int tmp = static_cast<int>(100.0f * volume * sounds[idx].max_volume);
        sounds[idx].sound->setVolume(tmp);
    }

    if (!sounds[idx].prev_play)
    {
        sounds[idx].sound->play();
        sounds[idx].prev_play = true;
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
        if (sounds[idx].prev_play)
        {
            sounds[idx].sound->stop();
            sounds[idx].prev_play = false;
        }
        return;
    }

    sounds[idx].sound->setPitch(pitch);

    if (!sounds[idx].prev_play)
    {
        sounds[idx].sound->play();
        sounds[idx].prev_play = true;
    }
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
