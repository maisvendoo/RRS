//------------------------------------------------------------------------------
//
//      Timer for execute periodic actions synchronously with ODE solving
//      (c) maisvendoo, 09/05/2019
//
//------------------------------------------------------------------------------

#ifndef     TIMER_H
#define     TIMER_H

#include    <QObject>

#include    "device-export.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Timer : public QObject
{
    Q_OBJECT

public:

    Timer(double timeout = 0.1, bool first_process = true, QObject *parent = Q_NULLPTR);

    ~Timer();

    /// Step (excecuted in integraion step)
    void step(double t, double dt);

    /// Start timer
    void start();

    /// Stop timer
    void stop();

    /// Reset timer
    void reset();

    /// Check is timer started
    bool isStarted() const;

    /// Set timeout
    void setTimeout(double timeout);

    /// Get time
    double getTime();

    /// Get timeout
    double getTimeout();

    void firstProcess(bool first_process);

signals:

    /// Signal for actions exectute
    void process();

private:

    /// Time counter
    double  tau;

    /// Timer  timeout
    double  timeout;

    /// First timer action
    bool    first_process;

    bool    fprocess_prev;

    /// is started flag
    bool    is_started;
};

#endif // TIMER_H
