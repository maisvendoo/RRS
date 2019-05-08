#ifndef     TIMER_H
#define     TIMER_H

#include    <QObject>

class Timer : public QObject
{
    Q_OBJECT

public:

    Timer(double timeout = 0.1, bool first_process = true, QObject *parent = Q_NULLPTR);

    ~Timer();

    void step(double t, double dt);

    void start();

    void stop();

    void reset();

    bool isStarted() const;

signals:

    void process();

private:

    double  tau;

    double  timeout;

    bool    first_process;

    bool    is_started;
};

#endif // TIMER_H
