#ifndef     KEYS_CONTROL_H
#define     KEYS_CONTROL_H

#include    <QThread>
#include    <QSharedMemory>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class KeysControl : public QObject
{
    Q_OBJECT

public:

    KeysControl(QObject *parent = Q_NULLPTR);

    ~KeysControl();

    void start();

signals:

    void sendDataToTrain(QByteArray data);

private:

    bool    is_started;

    QSharedMemory   keys_data;
    QThread         keys_thread;

private slots:

    void process();
};

#endif // KEYS_CONTROL_H
