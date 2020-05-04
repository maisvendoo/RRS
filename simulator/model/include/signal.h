#ifndef     SIGNAL_H
#define     SIGNAL_H

#include    <QObject>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Signal : public QObject
{
    Q_OBJECT

public:

    Signal(QObject *parent = Q_NULLPTR);

    virtual ~Signal();

protected:


};

#endif // SIGNAL_H
