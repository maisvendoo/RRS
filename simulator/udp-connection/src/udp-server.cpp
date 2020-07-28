#include    "udp-server.h"

UdpServer::UdpServer(QObject* parent) : QObject(parent)
{
    qRegisterMetaType<udp_server_data_t>();
}

UdpServer::~UdpServer()
{

}

bool UdpServer::isConnected() const
{
    return true;
}

void UdpServer::init(QString &cfg_path)
{
    load_config(cfg_path);

    udpSocket = new QUdpSocket();
    udpSocket->bind(QHostAddress::LocalHost, port);

    connect(udpSocket, &QUdpSocket::readyRead,
            this, &UdpServer::receive);
}

void UdpServer::setServerData(udp_server_data_t &data)
{
    server_data = data;
}

void UdpServer::receive()
{
    QByteArray recv_data = udpSocket->readAll();

    if (recv_data.at(0) == 1)
    {
        QByteArray raw_data = server_data.serialize();
        udpSocket->write(raw_data);
    }
}

void UdpServer::load_config(QString &path)
{
    CfgReader cfg;
    cfg.load(path);
    cfg.getInt("UdpServer", "Port", port);
}
