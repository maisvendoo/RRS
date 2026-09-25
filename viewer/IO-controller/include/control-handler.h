#ifndef     CONTROL_HANDLER_H
#define     CONTROL_HANDLER_H

#include    <QObject>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ControlHandler : public QObject
{
    Q_OBJECT

public:

    explicit ControlHandler(QObject *parent = nullptr) : QObject(parent)
    {

    }

    virtual ~ControlHandler() = default;

signals:

    void sigSendControlCommand(const QByteArray &data);

protected:


};

#endif
