#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "network-data-types.h"
#include "network-export.h"

#include <QTcpSocket>
#include <QTimer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct tcp_config_t
{
    QString host_addr = "127.0.0.1";
    quint16 port = 1992;
    int reconnect_interval = 500;
    bool show_server_addr = true;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class NETWORK_EXPORT TcpClient : public QObject
{
    Q_OBJECT

public:

    TcpClient(QObject *parent = nullptr);

    ~TcpClient();

    bool init(const tcp_config_t &tcp_config);

    void sendRequest(StructureType stype, double update_interval = 0.0);

    void sendSwitchCommand(QByteArray switch_command);

    void sendSignalCommand(QByteArray signal_command);

    void sendBuildRouteCommand(QByteArray route_command);

    void sendTrainRouteCommand(QByteArray route_command);

    void sendShuntingRouteCommand(QByteArray route_command);

    void sendVehicleControl(QByteArray vehicle_control_by_keyboard);

    void sendNewTrainName(int train_idx, const QString &new_name);

    void sendSimSpeedCommand(int speed_factor);

    bool isConnected() const;

signals:

    void connected();

    void disconnected();

    void setRouteInfo(QByteArray &route_info);

    void setTopologyData(QByteArray &topology_data);

    //void setTopologyState(QByteArray &topology_state);

    void setSwitchState(QByteArray &sw_state);

    void setTrajBusyState(QByteArray &busy_state);

    void setSignalsData(QByteArray &signals_data);

    //void setSignalsState(QByteArray &signals_state);

    void updateSignal(QByteArray signal_data);

    //void setPlayersInfo(QByteArray &players_info);

    void setPlayersUpdate(QByteArray &players_update);

    void setVehiclesInfo(QByteArray &vehicles_info);

    void setTrainInfo(QByteArray &data);

    void setVehiclesPositions(QByteArray &vehicles_pos);

    void setVehiclesData(QByteArray &vehicles_data);

    void setVehicleControlled(QByteArray &vehicle_controlled);

    void sendLogMessage(QString msg);

    void connectionAbandoned();

private:

    QTcpSocket *socket = nullptr;

    QTimer *connectionTimer = nullptr;

    tcp_config_t tcp_config;

    network_data_t received_data;

    QByteArray recvBuff;

    uint32_t wait_data_size = 0;

    bool is_first_data = true;

    QDataStream     in;

    int connection_attempts = 0;
    static constexpr int MAX_CONNECTION_ATTEMPTS = 60;

    bool canSend() const;

    void connectToServer(const tcp_config_t &tcp_config);

    void process_received_data(network_data_t &net_data);

public slots:

    void slotConnect();

    void slotDisconnect();

    void slotOnConnectionTimeout();

    void slotReceive();

    void slotAcceptError(QAbstractSocket::SocketError error);
};

#endif
