#ifndef		SOUND_MANAGER_H
#define		SOUND_MANAGER_H

#include    "sound-export.h"

#include    <QObject>
#include    <QMap>

#include    "asound.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SOUND_MANAGER_EXPORT SoundManager : public QObject
{
    Q_OBJECT

public:

    SoundManager(QObject *parent = Q_NULLPTR);

    ~SoundManager();

private:

    QMap<QString, ASound *> sounds;

public slots:

    void attachSound(const QString &name, const QString &path);
};

#endif // SOUND_MANAGER_H
