#ifndef     UDP_SERVER_H
#define     UDP_SERVER_H

#include    <QUdpSocket>
#include    <QNetworkDatagram>

#include    "CfgReader.h"
#include    "udp-data-struct.h"

Q_DECLARE_METATYPE(udp_server_data_t);

class UdpServer : public QObject
{
    Q_OBJECT

public:
    UdpServer(QObject *parent = Q_NULLPTR);

    ~UdpServer();

    void init(const QString& cfg_path);

    bool isConnected();

    void setServerData(udp_server_data_t &data);

//    void sendServerData();

public slots:

    void readPendingDatagrams();

    void slotConnected();

private:
    QUdpSocket *serverSocket;

    udp_server_data_t server_data;

    QHostAddress host;

    unsigned short port;

    void load_config(const QString& path);
};

#endif // UDPSERVER_H
