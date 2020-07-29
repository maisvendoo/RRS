#include "udp-client.h"

UdpClient::UdpClient()
{

}

UdpClient::~UdpClient()
{

}

void UdpClient::init(QString &cfg_path)
{
    load_config(cfg_path);

    clientSocket = new QUdpSocket();
    clientSocket->bind(QHostAddress::LocalHost, port);

    connect(clientSocket, &QUdpSocket::readyRead,
            this, &UdpClient::receive);
}

bool UdpClient::isConnected()
{
    if (clientSocket == Q_NULLPTR)
        return false;

    return  clientSocket->state() == QUdpSocket::ConnectedState;
}

void UdpClient::load_config(QString &path)
{
    CfgReader cfg;
    cfg.load(path);
    cfg.getInt("UdpServer", "SPort", port);
}

void UdpClient::receive()
{
    QByteArray recv_data = clientSocket->readAll();

    if(recv_data.at(0) == 1)
    {
        client_data.deserialize(recv_data);
    }
}
