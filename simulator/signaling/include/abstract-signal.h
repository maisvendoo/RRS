#ifndef     ABSTRACT_SIGNAL_H
#define     ABSTRACT_SIGNAL_H

#include    <QObject>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Signal : public QObject
{
    Q_OBJECT

public:

    Signal(QObject *parent = Q_NULLPTR);

    ~Signal();

protected:


};

#endif // ABSTRACT_SIGNAL_H
