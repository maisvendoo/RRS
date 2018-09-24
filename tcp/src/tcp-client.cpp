#include    "tcp-client.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TcpClient::TcpClient(QObject *parent) : QObject(parent)
  , socket(Q_NULLPTR)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TcpClient::~TcpClient()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::init(const tcp_client_config_t &tcp_client_config)
{
    this->tcp_client_config = tcp_client_config;

    socket = new QTcpSocket(this);
    in.setDevice(socket);
    in.setVersion(QDataStream::Qt_4_0);

    timerConnector = new QTimer(this);
    timerConnector->setInterval(tcp_client_config.reconect_interval);

    connect(timerConnector, &QTimer::timeout,
            this, &TcpClient::onTimerConnector);

    connect(socket, &QTcpSocket::connected,
            this, &TcpClient::onConnection);

    connect(socket, &QTcpSocket::disconnected,
            this, &TcpClient::onDisconnection);

    connect(socket, &QTcpSocket::readyRead,
            this, &TcpClient::onReceive);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::start()
{
    timerConnector->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TcpClient::isConnected() const
{
    return socket->state() == QTcpSocket::ConnectedState;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::connectToServer()
{
    socket->abort();
    socket->connectToHost(tcp_client_config.address,
                          tcp_client_config.port,
                          QIODevice::ReadWrite);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendDataToServer(QByteArray data)
{
    socket->write(data);
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::onTimerConnector()
{
    if (!isConnected())
    {
        this->connectToServer();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::onReceive()
{
    incommingData = socket->readAll();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::onConnection()
{
    timerConnector->stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::onDisconnection()
{
    socket->abort();
    timerConnector->start();
}
