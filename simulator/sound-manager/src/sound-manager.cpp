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
    std::string path = fs.getSoundsDir() + fs.separator()
            + vehicle_name.toStdString() + fs.separator() + "sounds.xml";

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

            secNode = cfg.getNextSection();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SoundManager::attachSound(const QString &name, const QString &path)
{

}
