//------------------------------------------------------------------------------
//
//      Server to commect with viewer
//      (c) maisvendoo, 18/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Server to commect with viewer
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 18/12/2018
 */

#ifndef     SERVER_H
#define     SERVER_H

#include    <QTcpServer>

#include    "a-tcp-namespace.h"
#include    "server-data-struct.h"

class TcpServer;
class QTimer;
class ClientFace;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct clients_t
{
    ClientFace *client;

    clients_t()
        : client(Q_NULLPTR)
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Server Q_DECL_FINAL : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    Server(QObject *parent = Q_NULLPTR);

    /// Destructor
    ~Server();

    /// Initialization
    void init(quint16 port);

    /// Get data received from client
    QByteArray getReceivedData();

signals:

    /// Print message to log
    void logMessage(QString msg);

public slots:

    /// Send data to client
    void sendDataToClient(QByteArray data);

private:        

    /// Server object
    TcpServer   *server;

    /// Connected clients objects
    clients_t   clients;

private slots:

    /// Perform when client authorized
    void clientAuthorized(ClientFace *clnt);

    /// Perform when client disconnected
    void clientDisconnected(ClientFace *clnt);
};

#endif // SERVER_H
