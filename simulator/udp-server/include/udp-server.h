#ifndef     UDP_SERVER_H
#define     UDP_SERVER_H

#include    <QObject>
#include    <QMetaType>
#include    <QUdpSocket>

#include    "udp-data-struct.h"
#include    "CfgReader.h"

Q_DECLARE_METATYPE(udp_server_data_t);

class UdpServer : public QObject
{
    Q_OBJECT

public:
    UdpServer(QObject *parent = Q_NULLPTR);

    ~UdpServer();

    void initSocket();

public slots:
    void getServerData(udp_server_data_t &data);

    void load_config(CfgReader &cfg);

private:
    QUdpSocket *udpSocket;

    udp_server_data_t server_data;

    int port;
};

#endif // UDPSERVER_H
