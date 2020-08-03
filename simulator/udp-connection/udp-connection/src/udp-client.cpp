#include "udp-client.h"

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

    clientSocket->connectToHost(host, port);

    connect(clientSocket, &QUdpSocket::readyRead,
            this, &UdpClient::receive);    
}

bool UdpClient::isConnected()
{
    if (clientSocket == Q_NULLPTR)
        return false;

    return  clientSocket->state() == QUdpSocket::ConnectedState;
}

void UdpClient::sendData(const QByteArray& data)
{
    clientSocket->write(data);
    clientSocket->flush();
}

void UdpClient::load_config(const QString &path)
{
    CfgReader cfg;
    cfg.load(path);

    QString host_str;
    cfg.getString("UdpClient", "HostAddr", host_str);
    host.setAddress(host_str);

    int port_int;
    cfg.getInt("UdpClient", "Port", port_int);
    port = static_cast<unsigned short>(port_int);
}

void UdpClient::receive()
{
    QByteArray recv_data = clientSocket->readAll();

    if (recv_data.isEmpty())
        return;

    if(recv_data.at(0) == 1)
    {
        client_data.deserialize(recv_data);
    }
}
