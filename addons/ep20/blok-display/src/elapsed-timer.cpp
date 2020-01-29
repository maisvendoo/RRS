#include    "elapsed-timer.h"

#include    <QElapsedTimer>
#include    <QTimer>
#include    <QEventLoop>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElapsedTimer::ElapsedTimer(QObject *parent) : QObject(parent)
  , is_started(false)
  , interval(0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElapsedTimer::~ElapsedTimer()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElapsedTimer::setInterval(quint64 interval)
{
    this->interval = interval;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElapsedTimer::start()
{
    is_started = true;
    connect(&thread, &QThread::started, this, &ElapsedTimer::loop);
    thread.start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElapsedTimer::loop()
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ElapsedTimer::process);
    timer->setTimerType(Qt::PreciseTimer);
    timer->start(static_cast<int>(interval));   
    timer->moveToThread(&thread);

    QEventLoop eventLoop;

    while (is_started)
    {
        eventLoop.processEvents();
    }
}
