#include	"sound-manager.h"
#include    "CfgReader.h"
#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SoundManager::SoundManager(QObject *parent) : QObject(parent)
{
    AListener listener = AListener::getInstance();
    Q_UNUSED(listener)
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
void SoundManager::loadSounds(const QString &vehicle_name)
{
    FileSystem &fs = FileSystem::getInstance();

    std::string soundsDir = fs.getSoundsDir() + fs.separator()
            + vehicle_name.toStdString();

    std::string path = soundsDir + fs.separator() + "sounds.xml";

    CfgReader cfg;

    if (cfg.load(QString(path.c_str())))
    {
        QDomNode secNode = cfg.getFirstSection("Sound");

        while (!secNode.isNull())
        {
            sound_config_t sound_config;

            cfg.getString(secNode, "Name", sound_config.name);
            cfg.getString(secNode, "Path", sound_config.path);
            cfg.getInt(secNode, "InitVolume", sound_config.init_volume);
            cfg.getInt(secNode, "MaxVolume", sound_config.max_volume);

            double tmp;
            cfg.getDouble(secNode, "InitPitch", tmp);
            sound_config.init_pitch = static_cast<float>(tmp);

            cfg.getBool(secNode, "Loop", sound_config.loop);
            cfg.getBool(secNode, "PlayOnStart", sound_config.play_on_start);

            sound_config.sound = new ASound(QString((soundsDir + fs.separator()).c_str())
                                            + sound_config.path);

            if (sound_config.sound->getLastError().isEmpty())
            {
                sound_config.sound->setVolume(sound_config.init_volume);
                sound_config.sound->setPitch(sound_config.init_pitch);
                sound_config.sound->setLoop(sound_config.loop);

                if (sound_config.play_on_start)
                    sound_config.sound->play();

                sounds.insert(sound_config.name, sound_config);
            }

            secNode = cfg.getNextSection();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::attachSound(const QString &name, const QString &path)
{
    Q_UNUSED(name)
    Q_UNUSED(path)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::play(QString name)
{
    auto it = sounds.find(name);

    if (it.key() == name)
        it.value().sound->play();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::stop(QString name)
{
    auto it = sounds.find(name);

    if (it.key() == name)
        it.value().sound->stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::setVolume(QString name, int volume)
{
    auto it = sounds.find(name);

    if (it.key() == name)
    {
        if (volume < 0)
            it.value().sound->setVolume(0);

        if (volume <= it.value().max_volume)
            it.value().sound->setVolume(volume);
        else
            it.value().sound->setVolume(it.value().max_volume);

        if (volume > 0)
        {
            if (!it.value().sound->isPlaying())
                it.value().sound->play();
        }
        else
        {
            it.value().sound->stop();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::setPitch(QString name, float pitch)
{
    auto it = sounds.find(name);

    if (it.key() == name)
    {
        it.value().sound->setPitch(pitch);

        if (pitch < 0.1f)
            it.value().sound->stop();
        else
        {
            if (!it.value().sound->isPlaying())
                it.value().sound->play();
        }
    }
}
