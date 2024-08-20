#ifndef     TCP_SERVER_H
#define     TCP_SERVER_H

#include    <QTcpServer>
#include    <network-export.h>
#include    <network-data-types.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class NETWORK_EXPORT TcpServer : public QObject
{
    Q_OBJECT

public:

    TcpServer(QObject *parent = Q_NULLPTR);

    ~TcpServer();

    bool init(QString cfg_path);

private:

    quint16 port = 1992;

    QTcpServer *server = Q_NULLPTR;

    std::vector<client_data_t> clients_data;

    int getClientDataBySocket(QTcpSocket *socket);

public slots:

    void slotNewConnection();

    void slotClientDisconnected();
};

#endif
