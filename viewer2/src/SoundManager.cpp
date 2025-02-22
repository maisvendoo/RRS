#include "SoundManager.h"
#include "CfgReader.h"
#include "Logger.h"
#include "filesystem.h"
#include <al.h>
#include <alc.h>

SoundManager::SoundManager()
    : device(nullptr)
    , context(nullptr)
{
    init();
}

SoundManager::~SoundManager()
{
    if (context)
    {
        alcDestroyContext(context);
    }

    if (device)
    {
        alcCloseDevice(device);
    }
}

void SoundManager::init()
{
    LOG_INFO("========================== SoundManager initialization ==========================");

    double tmp_volume = 1.0;
    int tmp_max_sources = 65535;

    FileSystem& fs = FileSystem::getInstance();
    QString cfg_path = QString(fs.getConfigDir().c_str()) + fs.separator() + "sound-settings.xml";
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Settings";

        cfg.getDouble(secName, "Volume", tmp_volume);
        cfg.getInt(secName, "MaxSources", tmp_max_sources);
    }
    ALfloat volume = static_cast<ALfloat>(std::max(0.0, std::min(1.0, tmp_volume)));
    ALCint max_sources = static_cast<ALCint>(std::max(1, std::min(1000000, tmp_max_sources)));

    device = alcOpenDevice(nullptr);
    ALCint context_attribute_list[2] = {ALC_MONO_SOURCES, max_sources};

    context = alcCreateContext(device, context_attribute_list);

    alcMakeContextCurrent(context);

    ALfloat pos[3] = {0.0f, 0.0f, 0.0f};
    alListenerfv(AL_POSITION, pos);

    ALfloat vel[3] = {0.0f, 0.0f, 0.0f};
    alListenerfv(AL_VELOCITY, vel);

    ALfloat ori[6] = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    alListenerfv(AL_ORIENTATION, ori);

    alListenerf(AL_GAIN, volume);

    LOG_INFO("Volume: %.3f", volume);
    LOG_INFO("Sources: %d", max_sources);
    LOG_INFO("=========================== Initialization successful ===========================");
}
