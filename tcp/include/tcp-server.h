//------------------------------------------------------------------------------
//
//      TCP-server
//      (c) maisvendoo, 18/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief TCP-server
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 18/09/2018
 */

#ifndef     TCP_SERVER_H
#define     TCP_SERVER_H

#include    "tcp-export.h"
#include    "tcp-config.h"
#include    "client.h"

#include    <QObject>
#include    <QTcpServer>
#include    <QTcpSocket>

/*!
 * \class
 * \brief TCP server
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TCP_EXPORT TcpServer : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    explicit TcpServer(QObject *parent = Q_NULLPTR);
    /// Destructor
    virtual ~TcpServer();

    /// Server initialization
    bool init();
    /// Server startup
    bool start();

signals:

    /// Send received data to train's model
    void sendDataToTrain(QByteArray data);

    /// Print message to log
    void logMessage(QString msg);

private:

    /// Server object
    QTcpServer      *server;
    /// Server configuration
    tcp_sever_config_t    tcp_config;
    /// List of connected clients
    QMap<QTcpSocket *, Client *> clients;

private slots:

    /// Actions client after connection
    void onClientConnection();
    /// Actions after data receive
    void onReceive();
    /// Actions after clinet disconnection
    void onClientDisconnection();
};

#endif // TCP_SERVER_H
