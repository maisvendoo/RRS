//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief
 * \copyright
 * \author
 * \date
 */

#ifndef     SERVER_H
#define     SERVER_H

#include    <QTcpServer>

#include    "a-tcp-namespace.h"
#include    "server-data-struct.h"

class TcpServer;
class QTimer;
class ClientFace;
class PProcess;

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

    Server(QObject *parent = Q_NULLPTR);

    ~Server();

    void init(quint16 port);

    QByteArray getReceivedData();

signals:

    void logMessage(QString msg);

public slots:

    void sendDataToClient(QByteArray data);

private:

    PProcess    *process;

    QTimer      *timer;

    TcpServer   *server;

    clients_t   clients;

private slots:

    void clientAuthorized(ClientFace *clnt);

    void clientDisconnected(ClientFace *clnt);
};

#endif // SERVER_H
