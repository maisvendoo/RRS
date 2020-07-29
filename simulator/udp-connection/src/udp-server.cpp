#include    "udp-server.h"

UdpServer::UdpServer(QObject* parent) : QObject(parent)
{
    qRegisterMetaType<udp_server_data_t>();
}

UdpServer::~UdpServer()
{

}

void UdpServer::init(QString &cfg_path)
{
    load_config(cfg_path);

    serverSocket = new QUdpSocket();
    serverSocket->bind(QHostAddress::LocalHost, port);

    connect(serverSocket, &QUdpSocket::readyRead,
            this, &UdpServer::receive);
}

bool UdpServer::isConnected()
{
    if (serverSocket == Q_NULLPTR)
        return false;

    return serverSocket->state() == QUdpSocket::ConnectedState;
}

void UdpServer::setServerData(udp_server_data_t &data)
{
    server_data = data;
}

void UdpServer::receive()
{
    QByteArray recv_data = serverSocket->readAll();

    if (recv_data.at(0) == 1)
    {
        QByteArray raw_data = server_data.serialize();
        serverSocket->write(raw_data);
    }
}

void UdpServer::load_config(QString &path)
{
    CfgReader cfg;
    cfg.load(path);
    cfg.getInt("UdpServer", "Port", port);
}
