//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     TCP_CLIENT_H
#define     TCP_CLIENT_H

#include    "tcp-config.h"
#include    "tcp-export.h"

#include    <QObject>
#include    <QTcpSocket>
#include    <QDataStream>
#include    <QTimer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TCP_EXPORT TcpClient : public QObject
{
    Q_OBJECT

public:

    explicit TcpClient(QObject *parent = Q_NULLPTR);
    virtual ~TcpClient();

    void init(const tcp_client_config_t &tcp_client_config);
    void start();

    bool isConnected() const;

public slots:

    void connectToServer();

    void sendDataToServer(QByteArray data);

private:

    QTcpSocket  *socket;

    QTimer      *timerConnector;

    QDataStream in;

    QByteArray  incommingData;

    tcp_client_config_t tcp_client_config;

private slots:

    void onTimerConnector();

    void onReceive();

    void onConnection();

    void onDisconnection();
};

#endif // TCP_CLIENT_H
