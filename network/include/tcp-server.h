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

    void setTopologyData(const QByteArray &topology_data)
    {
        this->topology_data = topology_data;
    }

private:

    quint16 port = 1992;

    QTcpServer *server = Q_NULLPTR;

    std::vector<client_data_t> clients_data;

    QByteArray topology_data;

    int getClientDataBySocket(QTcpSocket *socket);

    void process_client_request(client_data_t &client_data);

    void send_topology_data(client_data_t &client_data);

public slots:

    void slotNewConnection();

    void slotClientDisconnected();

    void slotReceive();
};

#endif
