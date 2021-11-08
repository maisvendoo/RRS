#include "film-server.h"

#include    "CfgReader.h"
#include <filesystem.h>



//-----------------------------------------------------------------------------
// Конструктор
//-----------------------------------------------------------------------------
FilmServer::FilmServer(QObject *parent)
    : QTcpServer(parent)
{
    loadCfg_();

    client_ = new QTcpSocket(this);

//    timerSend_ = new QTimer();
//    connect(timerSend_, &QTimer::timeout, this, &FilmServer::slotSendDataClient);
//    timerSend_->setInterval(tcp_conf_.data_send_interval);
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void FilmServer::start()
{
    connect(this, &FilmServer::newConnection, this, &FilmServer::slotClientAutorized);

    if (!this->isListening())
    {
        if(this->listen(QHostAddress::Any, tcp_conf_.port))
        {
            qDebug() << "Listening to port connection: " << tcp_conf_.port;
        }
        else
        {
            qDebug() << "Listening to the communication port " << tcp_conf_.port << " is not active";
        }
    }
}



void FilmServer::loadCfg_()
{
    CfgReader cfg;

    FileSystem &fs = FileSystem::getInstance();
    QString full_path = QString(fs.getConfigDir().c_str()) + fs.separator() + "film-tcp-config.xml";

    if (cfg.load(full_path))
    {
        QString sectionName = "TcpConfig";
        cfg.getInt(sectionName, "DataSendInterval", tcp_conf_.data_send_interval);
        int port = 0;
        cfg.getInt(sectionName, "Port", port);
        tcp_conf_.port = static_cast<quint16>(port);
    }
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void FilmServer::slotClientAutorized()
{
    client_ = this->nextPendingConnection();

    qDebug() << "Energy Dispatcher status: " << client_->state() << ", local address: " << client_->peerAddress();

    //timerSend_->start();

    connect(client_, &QTcpSocket::disconnected, this, &FilmServer::slotClientDisconnected);
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void FilmServer::slotClientDisconnected()
{
    qDebug() << "Energy Dispatcher status: " << client_->state() << ", local address: " << client_->peerAddress();

    //timerSend_->stop();
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void FilmServer::slotSendDataClient(data_client_t data)
{
    if (client_ == Q_NULLPTR)
        return;

    if (client_->state() != QTcpSocket::ConnectedState)
        return;


    QByteArray arr;
    arr.resize(sizeof(data_client_t));

    arr = data.serialize();

    if (client_->isOpen())
    {
        client_->write(arr);
        client_->flush();
    }
}
