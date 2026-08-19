#ifndef     ELAPSED_TIMER_H
#define     ELAPSED_TIMER_H

#include    <QThread>
#include    <QTimer>

class ElapsedTimer : public QObject
{
    Q_OBJECT

public:

    ElapsedTimer(QObject *parent = nullptr);

    ~ElapsedTimer();

    void setInterval(quint64 interval);

    void start();

signals:

    void process();

private:

    bool    is_started;

    quint64 interval;

    QThread thread;

    QTimer *timer = new QTimer(this);

private slots:

    void loop();
};

#endif // ELAPSED_TIMER_H
