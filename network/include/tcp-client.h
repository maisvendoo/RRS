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
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class NETWORK_EXPORT TcpClient : public QObject
{
    Q_OBJECT

public:

    TcpClient(QObject *parent = Q_NULLPTR);

    ~TcpClient();

    bool init(const tcp_config_t &tcp_config);

    void sendRequest(StructureType stype, double update_interval = 0.0);

    void sendSwitchState(QString conn_name, int state_fwd, int state_bwd);

    void sendSignalState(QString conn_name, int sig_dir, bool open);

    void sendVehicleControl(QByteArray vehicle_control_by_keyboard);

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

    void setVehiclesPositions(QByteArray &vehicles_pos);

    void setVehiclesData(QByteArray &vehicles_data);

    void setVehicleControlled(QByteArray &vehicle_controlled);

    void sendLogMessage(QString msg);

private:

    QTcpSocket *socket = Q_NULLPTR;

    QTimer *connectionTimer = Q_NULLPTR;

    tcp_config_t tcp_config;

    network_data_t received_data;

    QByteArray recvBuff;

    qsizetype wait_data_size = 0;

    bool is_first_data = true;

    QDataStream     in;

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
