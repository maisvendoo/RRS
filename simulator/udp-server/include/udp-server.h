#ifndef     UDP_SERVER_H
#define     UDP_SERVER_H

#include    <QObject>
#include    <QMetaType>
#include    <QUdpSocket>

#include    "udp-data-struct.h"
#include    "CfgReader.h"
#include    "udp-client.h"

Q_DECLARE_METATYPE(udp_server_data_t);

class UdpServer : public QObject
{
    Q_OBJECT

public:
    UdpServer(QObject *parent = Q_NULLPTR);

    ~UdpServer();

    bool isConnected() const;

    void init(QString &cfg_path);

    void setServerData(udp_server_data_t &data);


private slots:

    void receive();

private:
    UdpClient *udpClient;

    QUdpSocket *udpSocket;

    udp_server_data_t server_data;

    int port;

    void load_config(QString &path);
};

#endif // UDPSERVER_H
