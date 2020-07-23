#include    "udp-server.h"

UdpServer::UdpServer(QObject* parent) : QObject(parent)
{
    qRegisterMetaType<udp_server_data_t>();
}

UdpServer::~UdpServer()
{

}

void UdpServer::initSocket()
{
    udpSocket = new QUdpSocket();
    udpSocket->bind(QHostAddress::LocalHost, port);

    connect(udpSocket, &QUdpSocket::readyRead,
            this, &UdpServer::getServerData);
}

void UdpServer::getServerData(udp_server_data_t &data)
{
    QByteArray byteData = data.serialize();
    server_data = data.deserialize(byteData);
}

void UdpServer::load_config(CfgReader& cfg)
{
    cfg.getInt("UdpServer", "Port", port);
}
