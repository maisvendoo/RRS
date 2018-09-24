//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     TCP_SERVER_H
#define     TCP_SERVER_H

#include    "tcp-export.h"
#include    "tcp-config.h"
#include    "client.h"

#include    <QObject>
#include    <QTcpServer>
#include    <QTcpSocket>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TCP_EXPORT TcpServer : public QObject
{
    Q_OBJECT

public:

    explicit TcpServer(QObject *parent = Q_NULLPTR);
    virtual ~TcpServer();

    bool init();
    bool start();

signals:

    void sendDataToTrain(QByteArray data);

private:

    QTcpServer      *server;

    tcp_config_t    tcp_config;

    QMap<QTcpSocket *, Client *> clients;

private slots:

    void onClientConnection();

    void onReceive();

    void onClientDisconnection();
};

#endif // TCP_SERVER_H
