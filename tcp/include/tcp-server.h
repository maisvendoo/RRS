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

private:

    QTcpServer      *server;

    tcp_config_t    tcp_config;

private slots:

    void onClientConnection();
};

#endif // TCP_SERVER_H
