#ifndef     ELAPSED_TIMER_H
#define     ELAPSED_TIMER_H

#include    <QThread>

class ElapsedTimer : public QObject
{
    Q_OBJECT

public:

    ElapsedTimer(QObject *parent = Q_NULLPTR);

    ~ElapsedTimer();

    void setInterval(quint64 interval);

    void start();

signals:

    void process();

private:

    bool    is_started;

    quint64 interval;

    QThread thread;

private slots:

    void loop();
};

#endif // ELAPSED_TIMER_H
