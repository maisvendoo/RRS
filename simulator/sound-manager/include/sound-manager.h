#ifndef		SOUND_MANAGER_H
#define		SOUND_MANAGER_H

#include    "sound-export.h"

#include    <QObject>
#include    <QMap>

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

    void loadSounds(const QString &vehicle_name);

private:

    QMap<QString, sound_config_t> sounds;

    void attachSound(const QString &name, const QString &path);
};

#endif // SOUND_MANAGER_H
