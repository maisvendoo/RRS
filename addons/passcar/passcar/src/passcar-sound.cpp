#include    "passcar.h"
#include    "filesystem.h"

#include    <QDir>

//------------------------------------------------------------------------------
// Инициализация звуков, воспроизведение их на нулевой громкости
//------------------------------------------------------------------------------
void PassCarrige::initSounds()
{
    getSoundList();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCarrige::soundStep()
{
    double speed = velocity * 3.6;
    QMap<int, QString>::const_iterator i = sounds.constBegin();
    QString soundPlay = "";

    while (i != sounds.constEnd())
    {
        if (speed >= i.key())
        {
            soundPlay = i.value();
        }
        if (speed < i.key())
        {
            break;
        }
        ++i;
    }

    playPasscarSound(soundPlay);
}

void PassCarrige::getSoundList()
{
    sounds.insert(5, "passcar_5_10");
    sounds.insert(10, "passcar_10_15");
    sounds.insert(15, "passcar_15_20");
    sounds.insert(20, "passcar_20_30");
    sounds.insert(30, "passcar_30_40");
    sounds.insert(40, "passcar_40_50");
    sounds.insert(50, "passcar_50_60");
    sounds.insert(60, "passcar_60_70");
    sounds.insert(70, "passcar_70_80");
    sounds.insert(80, "passcar_80_90");
    sounds.insert(90, "passcar_90_100");
    sounds.insert(100, "passcar_100_120");
    sounds.insert(120, "passcar_120_140");
}

void PassCarrige::playPasscarSound(QString sound_name)
{
    QMap<int, QString>::const_iterator i = sounds.constBegin();
    int volume;

    while (i != sounds.constEnd())
    {
        if (sound_name == i.value())
        {
            volume = 100;
        } else {
            volume = 0;
        }
        emit soundSetVolume(i.value(), volume);
        ++i;
    }
}
