#include    "elapsed-timer.h"

#include    <QElapsedTimer>
#include    <QTimer>
#include    <QEventLoop>

#include    "Journal.h"

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
    connect(timer, &QTimer::timeout, this, &ElapsedTimer::process, Qt::DirectConnection);
    timer->setTimerType(Qt::PreciseTimer);
    timer->start(static_cast<int>(interval));

    Journal::instance()->info(QString("Started simulation timer with interval %1").arg(interval));

    QEventLoop eventLoop;

    while (is_started)
    {
        eventLoop.processEvents();
    }
}
