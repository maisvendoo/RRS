#include "udp-client.h"

#include <QNetworkProxy>

UdpClient::UdpClient()
{

}

UdpClient::~UdpClient()
{

}

void UdpClient::init(const QString &cfg_path)
{
    load_config(cfg_path);

    clientSocket = new QUdpSocket();

    clientSocket->bind(QHostAddress::LocalHost, client_port);

    if (this->isConnected())
    {
        emit authorized();
    }

    connect(clientSocket, &QUdpSocket::readyRead,
            this, &UdpClient::readPendingDatagrams);
}

bool UdpClient::isConnected()
{
    if (clientSocket == Q_NULLPTR)
    {
        return false;
    }

    if (clientSocket->state() == QUdpSocket::BoundState)
    {
        return true;
    }

    else
    {
        emit disconnectedFromServer();
        return false;
    }
}

void UdpClient::sendData(const QByteArray& data)
{
    clientSocket->writeDatagram(data, server_host, server_port);
}

void UdpClient::setNoProxy(bool no_proxy)
{
    if (no_proxy)
        clientSocket->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
}

void UdpClient::load_config(const QString &path)
{
    CfgReader cfg;
    cfg.load(path);

    QString host_str;
    int port_int;

    cfg.getString("UdpServer", "HostAddr", host_str);
    server_host.setAddress(host_str);

//    cfg.getString("UdpClient", "HostAddr", host_str);
//    client_host.setAddress(host_str);

    cfg.getInt("UdpServer", "Port", port_int);
    server_port = static_cast<unsigned short>(port_int);

//    cfg.getInt("UdpClient", "Port", port_int);
//    client_port = static_cast<unsigned short>(port_int);
}

void UdpClient::readPendingDatagrams()
{
    while (clientSocket->hasPendingDatagrams())
    {
        incomingData_.resize(static_cast<int>(clientSocket->pendingDatagramSize()));

        clientSocket->readDatagram(incomingData_.data(), incomingData_.size(), &server_host, &server_port);

        if (!incomingData_.isEmpty())
        {
            client_data.deserialize(incomingData_);
        }
    }
}
