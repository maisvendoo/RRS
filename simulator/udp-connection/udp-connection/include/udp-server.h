#ifndef     UDP_SERVER_H
#define     UDP_SERVER_H

#include    <QUdpSocket>

#include    "CfgReader.h"
#include    "udp-data-struct.h"

Q_DECLARE_METATYPE(udp_server_data_t);

class UdpServer : public QObject
{
    Q_OBJECT

public:
    UdpServer(QObject *parent = Q_NULLPTR);

    ~UdpServer();

    void init(QString &cfg_path);

    bool isConnected();

    void setServerData(udp_server_data_t &data);

//    void sendServerData();

public slots:

    void receive();

private:
    QUdpSocket *serverSocket;

    udp_server_data_t server_data;

    int port;

    void load_config(QString &path);
};

#endif // UDPSERVER_H
