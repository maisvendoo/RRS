#ifndef     TCP_SERVER_H
#define     TCP_SERVER_H

#include    <QTcpServer>
#include    <QMap>
#include    <QSet>
#include    <QMutex>
#include    <QPair>
#include    <QList>
#include    <QAbstractSocket>
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
    void setStationsData(QByteArray data)
    {
        this->stations_data = data;
    }

    /// Забрать накопленные пакеты управления (потокобезопасно).
    /// Физика забирает их каждый тик: queued-доставка сигналов из
    /// сетевого потока не работает надёжно, поэтому обмен через
    /// мьютекс-буфер
    QList<QPair<int, QByteArray>> takePendingControl()
    {
        QMutexLocker lock(&pending_control_mutex);
        QList<QPair<int, QByteArray>> out = pending_control;
        pending_control.clear();
        return out;
    }

    /// Снимок сериализованной топологии/сигналов для новых клиентов.
    /// сервер сети живёт в отдельном потоке, поэтому
    /// данные передаются слотами (queued), а не забираются из модели
    /// по ссылке из сетевого потока
    void updateTopologyData(QByteArray topology_data);
    void updateSignalsData(QByteArray signals_data);

    void updatePlayers(QByteArray players_data, double t);

    void updateVehiclesPos(QByteArray vehicles_pos, double t);

    void updateVehiclesState(QByteArray vehicles_state, double t);

    void updateVehicleControlled(QByteArray vehicles_state, int client_id, double t);

    void updateTrainsInfo(QByteArray trains_state);

    /// Рассылка снимка диагностики составов
    void updateDiagnostics(QByteArray diagnostics_data, double t);
    void updateTrainProfile(QByteArray profile_data, double t);

    /// Есть ли клиенты, запросившие обновление профилей поездов
    bool hasTrainProfileSubscribers() const
    {
        return !clients_for_train_profile_updates.empty();
    }

    /// Максимальные запрошенные дальности профиля назад/вперёд, м,
    /// по всем подписчикам
    void getTrainProfileExtents(double &backward_m, double &forward_m) const
    {
        backward_m = 4000.0;
        forward_m = 4000.0;
        for (auto client_socket : clients_for_train_profile_updates)
        {
            auto it = clients_data.find(client_socket);
            if (it == clients_data.end())
                continue;
            const client_data_t &client = it.value();
            if (client.profile_backward > backward_m)
                backward_m = client.profile_backward;
            if (client.profile_forward > forward_m)
                forward_m = client.profile_forward;
        }
    }

signals:

    // ВАЖНО: сервер сети живёт в отдельном потоке,
    // поэтому сигналы передают данные ПО ЗНАЧЕНИЮ - Qt не умеет ставить
    // в очередь аргументы-ссылки (QByteArray&)

    void requestTopologyData(QByteArray &topology_data);

    void requestSignalsData(QByteArray &signals_data);

    void sigSwitchCommand(QByteArray switch_command);
    void requestTopologyModules(QByteArray& modules_data);

    void sigSignalCommand(QByteArray signal_command);

    void sigBuildRouteCommand(QByteArray route_command);

    void sigTrainRouteCommand(QByteArray route_command);

    void sigShuntingRouteCommand(QByteArray route_command);

    void sigVehicleControl(QByteArray& control_data, int client_id);

    void sigResetVehicleControl(int client_id);

    void sigRenameTrain(int train_idx, QString new_name);

    void sigReverseTrain(int train_idx);

    void sigSetSimSpeed(int speed_factor);

    void sigSetVehicleControlCommand(int vehicle_idx, int cab_idx, uint16_t id, float value);

    /// Табельный номер клиента: автоназначение
    /// поезда при подключении и восстановление "зависшего" (п.6)
    void sigClientTabNumber(int client_id, int tab_number);

    /// Организатор: назначить поезду табельный номер игрока
    void sigSetTrainTab(int train_idx, int tab_number);

    /// Организатор: загрузить сейв сессии
    void sigLoadSession(QString path);

private:

    /// Потолок размера сетевого пакета (защита от мусора в канале, /)
    static constexpr quint32 MAX_PACKET_SIZE = 100u * 1024u * 1024u;

    quint16 port = 1992;

    QTcpServer *server = nullptr;

    int clients_last_id = 0;

    QMap<QTcpSocket*, client_data_t> clients_data;

    QSet<QTcpSocket*> clients_for_players_info_updates;

    QSet<QTcpSocket*> clients_for_topology_updates;

    QSet<QTcpSocket*> clients_for_signals_updates;

    QSet<QTcpSocket*> clients_for_topology_modules_updates;

    QSet<QTcpSocket*> clients_for_trains_updates;

    QSet<QTcpSocket*> clients_for_vehicles_pos_updates;

    QSet<QTcpSocket*> clients_for_vehicles_updates;

    QSet<QTcpSocket*> clients_for_vehicle_controlled_updates;

    QSet<QTcpSocket*> clients_for_diagnostics_updates;
    QSet<QTcpSocket*> clients_for_train_profile_updates;

    QByteArray recvBuff;

    QByteArray route_info;

    QByteArray vehicles_info;

    QByteArray stations_data;

    QByteArray vehicles_state;

    QByteArray trains_state;

    /// Кэш сериализованной топологии/сигналов для новых клиентов
    /// (обновляется моделью через слоты)
    QByteArray topology_data;

    QByteArray signals_data;

    uint32_t wait_data_size = 0;

    bool is_first_data = true;

    void process_client_request(client_data_t &client_data);

    void send_route_info(client_data_t &client_data);

    void send_topology_data(client_data_t &client_data);

    void send_topology_modules(client_data_t &client_data);

    //void send_topology_state(client_data_t &client_data);

    void send_signals_data(client_data_t &client_data);

    void send_stations_data(client_data_t &client_data);

    //void send_signals_state(client_data_t &client_data);

    void send_vehicles_info(client_data_t &client_data);

    void send_trains_info(client_data_t &client_data);

    /// Буфер пакетов управления от клиентов (сетевой поток пишет,
    /// физика забирает takePendingControl'ом)
    QMutex pending_control_mutex;
    QList<QPair<int, QByteArray>> pending_control;
    void send_data(QTcpSocket *client_socket, network_data_t& net_data);

    void remove_client(QTcpSocket* socket);

public slots:

    void slotNewConnection();

    void slotClientDisconnected();

    void slotReceive();

    /// Ошибки обмена с клиентом
    void slotSocketError(QAbstractSocket::SocketError socket_error);

    void slotSendSwitchState(QByteArray sw_state);

    void slotSendTrajBusyState(QByteArray busy_state);

    void slotSendTopologyModuleState(QByteArray module_state);

    void slotUpdateSignal(QByteArray signal_data);
};

#endif
