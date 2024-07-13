#ifndef		SOUND_MANAGER_H
#define		SOUND_MANAGER_H

#include    "sound-export.h"

#include    <QObject>
#include    <vector>

#include    "sound-signal.h"
#include    "sound-config.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SOUND_MANAGER_EXPORT SoundManager : public QObject
{
    Q_OBJECT

public:

    SoundManager(QObject *parent = Q_NULLPTR);

    ~SoundManager();

    std::vector<size_t> loadSounds(const QString &sounddir);

    size_t getSignalID(size_t idx);

    float getLocalPositionX(size_t idx);
    float getLocalPositionY(size_t idx);
    float getLocalPositionZ(size_t idx);

signals:

    void notify(const std::string msg);

private:

    std::vector<sound_config_t> sounds;

public slots:

    void setListenerPosition(float x, float y, float z);

    void setListenerVelocity(float x, float y, float z);

    void setListenerOrientation(float at_x, float at_y, float at_z, float up_x = 0.0f, float up_y = 0.0f, float up_z = 1.0f);

    void setSoundState(size_t idx, sound_state_t ss);

    void play(size_t idx);

    void stop(size_t idx);

    void setVolume(size_t idx, float volume);

    void setPitch(size_t idx, float pitch);

    void setPosition(size_t idx, float x, float y, float z);

    void setVelocity(size_t idx, float x, float y, float z);
};

#endif // SOUND_MANAGER_H
