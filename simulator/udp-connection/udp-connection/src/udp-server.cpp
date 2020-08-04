#include    "udp-server.h"

UdpServer::UdpServer(QObject* parent) : QObject(parent)
{
    qRegisterMetaType<udp_server_data_t>();
}

UdpServer::~UdpServer()
{

}

void UdpServer::init(const QString &cfg_path)
{
    load_config(cfg_path);

    serverSocket = new QUdpSocket();

    serverSocket->bind(host, port);

    connect(serverSocket, &QUdpSocket::readyRead,
            this, &UdpServer::readPendingDatagrams);

    connect(serverSocket, &QUdpSocket::connected,
            this, &UdpServer::slotConnected);
}

bool UdpServer::isConnected()
{
    if (serverSocket == Q_NULLPTR)
    {
        return false;
    }

    return serverSocket->state() == QUdpSocket::BoundState;
}

void UdpServer::setServerData(udp_server_data_t &data)
{
    server_data = data;
}

void UdpServer::readPendingDatagrams()
{
    while (serverSocket->hasPendingDatagrams())
    {
        serverSocket->writeDatagram(serverSocket->receiveDatagram());
//        QNetworkDatagram recv_data_net = serverSocket->receiveDatagram();
//        QByteArray recv_data = recv_data_net.data();

//        if (recv_data.isEmpty())
//            return;

//        if (recv_data.at(0) == 1)
//        {
//            QByteArray raw_data = server_data.serialize();
//            serverSocket->writeDatagram(raw_data, host, port);
//        }
    }
}

void UdpServer::slotConnected()
{
    int zu = 0;
}

void UdpServer::load_config(const QString &path)
{
    CfgReader cfg;
    cfg.load(path);

    QString host_str;
    cfg.getString("UdpServer", "HostAddr", host_str);
    host.setAddress(host_str);

    int port_int;
    cfg.getInt("UdpServer", "Port", port_int);
    port = static_cast<unsigned short>(port_int) ;
}
