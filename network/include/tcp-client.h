#ifndef     TCP_CLIENT_H
#define     TCP_CLIENT_H

#include    <QTcpSocket>
#include    <QTimer>
#include    <network-export.h>
#include    <network-data-types.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class NETWORK_EXPORT TcpClient : public QObject
{
    Q_OBJECT

public:

    TcpClient(QObject *parent = Q_NULLPTR);

    ~TcpClient();

    bool init(QString cfg_name);

    void sendRequest(StructureType stype);

private:

    QTcpSocket *socket = Q_NULLPTR;

    QTimer *connectionTimer = Q_NULLPTR;

    struct tcp_config_t
    {
        QString host_addr ="127.0.0.1";
        quint16 port = 1992;
        int reconnect_interval = 100;
    };

    tcp_config_t tcp_config;

    void connectToServer(const tcp_config_t &tcp_config);

public slots:

    void slotConnect();

    void slotDisconnect();

    void slotOnConnectionTimeout();

    void slotReceive();
};

#endif
