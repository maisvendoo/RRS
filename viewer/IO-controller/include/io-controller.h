#ifndef     IO_CONTROLLER_H
#define     IO_CONTROLLER_H

#include    <io-controller-export.h>
#include    <QObject>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class IOController : public QObject
{
    Q_OBJECT

public:

    IOController(QObject *parent = nullptr);

    ~IOController() = default;

protected:


};

#endif
