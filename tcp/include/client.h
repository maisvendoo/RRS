//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     CLIENT_H
#define     CLIENT_H

#include    <QObject>
#include    <QTcpSocket>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Client : public QObject
{
    Q_OBJECT

public:

    explicit Client(QObject *parent = Q_NULLPTR);
    virtual ~Client();

    void setSocket(QTcpSocket *socket);

protected:

    QString     name;
    qintptr     id;
    QTcpSocket  *socket;
};

#endif // CLIENT_H
