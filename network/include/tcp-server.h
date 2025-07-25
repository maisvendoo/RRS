#ifndef     TCP_SERVER_H
#define     TCP_SERVER_H

#include    <QTcpServer>
#include    <QMap>
#include    <QSet>
#include    <network-export.h>
#include    <network-data-types.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class NETWORK_EXPORT TcpServer : public QObject
{
    Q_OBJECT

public:

    TcpServer(QObject *parent = nullptr);

    ~TcpServer();

    bool init(QString cfg_path);

    void setRouteInfo(QByteArray data)
    {
        this->route_info = data;
    }
    void setVehiclesInfo(QByteArray data)
    {
        this->vehicles_info = data;
    }

    void updatePlayers(QByteArray players_data, double t);

    void updateVehiclesPos(QByteArray vehicles_pos, double t);

    void updateVehiclesState(QByteArray vehicles_state, double t);

    void updateVehicleControlled(QByteArray vehicles_state, int client_id, double t);

signals:

    void requestTopologyData(QByteArray &topology_data);

    void requestSignalsData(QByteArray &signals_data);

    void setSwitchState(QByteArray &switch_data);

    void openSignal(QByteArray signal_data);

    void closeSignal(QByteArray signal_data);

    void setVehicleControl(QByteArray &control_data, int client_id);

    void resetVehicleControl(int client_id);

private:

    quint16 port = 1992;

    QTcpServer *server = nullptr;

    int clients_last_id = 0;

    QMap<QTcpSocket*, client_data_t> clients_data;

    QSet<QTcpSocket*> clients_for_players_info_updates;

    QSet<QTcpSocket*> clients_for_topology_updates;

    QSet<QTcpSocket*> clients_for_signals_updates;

    QSet<QTcpSocket*> clients_for_vehicles_pos_updates;

    QSet<QTcpSocket*> clients_for_vehicles_updates;

    QSet<QTcpSocket*> clients_for_vehicle_controlled_updates;

    QByteArray route_info;

    QByteArray vehicles_info;

    QByteArray recvBuff;

    qsizetype wait_data_size = 0;

    bool is_first_data = true;

    client_data_t map_client;

    void process_client_request(client_data_t &client_data);

    void send_route_info(client_data_t &client_data);

    void send_topology_data(client_data_t &client_data);

    //void send_topology_state(client_data_t &client_data);

    void send_signals_data(client_data_t &client_data);

    //void send_signals_state(client_data_t &client_data);

    void send_vehicles_info(client_data_t &client_data);

public slots:

    void slotNewConnection();

    void slotClientDisconnected();

    void slotReceive();

    void slotSendSwitchState(QByteArray sw_state);

    void slotSendTrajBusyState(QByteArray busy_state);

    void slotUpdateSignal(QByteArray signal_data);
};

#endif
