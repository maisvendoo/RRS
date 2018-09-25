//------------------------------------------------------------------------------
//
//      TCP-client
//      (c) maisvendoo, 24/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief TCP-client
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 24/09/2018
 */

#ifndef     TCP_CLIENT_H
#define     TCP_CLIENT_H

#include    "tcp-config.h"
#include    "tcp-export.h"

#include    <QObject>
#include    <QTcpSocket>
#include    <QDataStream>
#include    <QTimer>

/*!
 * \class
 * \brief TCP-clinet
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TCP_EXPORT TcpClient : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    explicit TcpClient(QObject *parent = Q_NULLPTR);
    /// Destructor
    virtual ~TcpClient();

    /// Client connection initialization
    void init(const tcp_client_config_t &tcp_client_config);
    /// Client processing startup
    void start();

    /// Check is client connected to server
    bool isConnected() const;

public slots:

    /// Connection to server
    void connectToServer();
    /// Send data to server
    void sendDataToServer(QByteArray data);

private:

    /// Client's socket object
    QTcpSocket  *socket;
    /// Reconnection timer
    QTimer      *timerConnector;
    /// Data stream to transfer processing
    QDataStream in;
    /// Data, recieved from server
    QByteArray  incommingData;
    /// Client's connection config
    tcp_client_config_t tcp_client_config;

private slots:

    /// Action on reconnetion time interval
    void onTimerConnector();
    /// Action on data receive
    void onReceive();
    /// Actions on connection to server
    void onConnection();
    /// Actions on disconnect from server
    void onDisconnection();
};

#endif // TCP_CLIENT_H
