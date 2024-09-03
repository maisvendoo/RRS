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

    void setSimulatorData(QByteArray simulator_data)
    {
        this->simulator_data = simulator_data;
    }

signals:

    void setTopologyData(QByteArray &topology_data);

    void setSwitchState(QByteArray &switch_data);

    void setTrajBusyState(QByteArray &busy_data);

private:

    quint16 port = 1992;

    QTcpServer *server = Q_NULLPTR;

    std::vector<client_data_t> clients_data;

    QByteArray topology_data;

    QByteArray recvBuff;

    qsizetype wait_data_size = 0;

    bool is_first_data = true;

    QByteArray simulator_data;

    client_data_t map_client;

    int getClientDataBySocket(QTcpSocket *socket);

    void process_client_request(client_data_t &client_data);

    void send_topology_data(client_data_t &client_data);

    void send_simulator_data(client_data_t &client_data);

public slots:

    void slotNewConnection();

    void slotClientDisconnected();

    void slotReceive();

    void slotSendSwitchState(QByteArray sw_state);

    void slotSendTrajBusyState(QByteArray busy_state);
};

#endif
