#ifndef     TRIGGER_H
#define     TRIGGER_H

#include    <QObject>

class   Trigger : public QObject
{
    Q_OBJECT

public:

    Trigger(QObject *parent = Q_NULLPTR);

    ~Trigger();

    bool getState() const;

    void set();

    void reset();

signals:

    void soundPlay(QString name);

    void soundStop(QString name);

    void soundSetVolume(QString name, int volume);

    void soundSetPitch(QString name, float pitch);

private:

    bool state;
};

#endif // TRIGGER_H
