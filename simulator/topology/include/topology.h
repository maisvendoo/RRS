#ifndef     TOPOLOGY_H
#define     TOPOLOGY_H

#include    <QObject>
#include    <unordered_map>

#include    <topology-export.h>
#include    <topology-types.h>
#include    <vehicle-controller.h>
#include    <vehicle.h>
#include    <signals-data-types.h>
#include    <route-segment.h>

/*!
 * \class
 * \brief Класс, обеспечивающий расчет положения ПЕ на путевой структуре
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT Topology : public QObject
{
    Q_OBJECT

public:

    Topology(QObject *parent = nullptr);

    ~Topology();

    /// Загрузка топологии ж/д полигона
    bool load(QString route_dir, bool solve_errors = true);

    /// Инициализация поезда
    bool addTrain(const topology_pos_t &tp, std::vector<Vehicle *> *vehicles);

    /// Вернуть контроллер конкретной ПЕ
    VehicleController *getVehicleController(size_t idx);

    /// Хэш-таблица указателей на вайкл контроллеры по указателю на вайкл
    /// (для удобства смены индекса поезда у контролов из поезда)
    std::unordered_map<Vehicle *, VehicleController *> vc_table;

    /// Нахождение пути в графе траекторий
    route_segment_t find_route(Trajectory *start_traj,
                               Trajectory *target_traj,
                               int dir);

    /// Шаг симуляции
    void step(double t, double dt);

    QByteArray serialize();

    void deserialize(QByteArray &data);

    traj_list_t *getTrajectoriesList()
    {
        return &traj_list;
    }

    const traj_list_t* getTrajectoriesList() const
    {
        return &traj_list;
    }

    conn_list_t *getConnectorsList()
    {
        return &switches;
    }

    const conn_list_t* getConnectorsList() const
    {
        return &switches;
    }

    topology_stations_list_t *getStationsList()
    {
        return &stations;
    }

    signals_data_t *getSignalsData()
    {
        return &signals_data;
    }

    QString getRouteName() const
    {
        return route_name;
    }

signals:

    void sendSwitchState(QByteArray sw_data);

    void sendTrajBusyState(QByteArray busy_data);

    void sigSetOpenSignalsQueue(QStringList conn_list, int dir);

    void sigChangeTrajStateByTrain(int train_idx, bool is_busy, QString traj_name);

private:

    /// Контейнер данных по всем траекториям на полигоне
    traj_list_t     traj_list;

    /// Контейнер изостыков
    conn_list_t     joints;

    /// Контейнер стрелок
    conn_list_t     switches;

    /// Контейнер контроллеров ПЕ
    std::vector<VehicleController *> vehicle_control;

    /// Сипсок станций
    topology_stations_list_t stations;

    /// Контейнер сигналов
    signals_data_t  signals_data;

    /// Название маршрута
    QString route_name = "";

    /// Получить список имен всех имеющихся траекторий
    QStringList getTrajNamesList(QString route_dir);

    /// Загрузка конфиг-файлов модулей путевой инфраструктуры
    std::vector<std::vector<module_cfg_t>> load_topology_configs(QString route_path);

    /// Загрузка топологии
    bool load_topology(QString route_dir);

    /// Загрузка сигналов (пока ограничиваюсь проходными)
    void load_signals(CfgReader &cfg, QDomNode secNode, Connector *conn);

    /// Загрузка списка станций
    bool load_stations(QString route_dir);

    /// Получение название маршрута из конфига описания
    void get_route_name(QString route_dir);

    void serialize_connector_name(QDataStream &stream, Connector *conn);

    Connector *deserialize_traj_connectors(QDataStream &stream, conn_list_t &conn_list) const;

    /// Установка стрелок по маршруту
    bool set_switchs_by_route(const route_segment_t &route, int dir);

    /// Октрытие попутных сигналов по маршруту
    bool open_route_signals(const route_segment_t &route, int dir, QStringList &conn_list);

public slots:

    void slotGetSwitchState(QByteArray &switch_data);

    void slotSwitchCommand(QByteArray& switch_data);

    void slotSignalCommand(QByteArray& signal_data);

    void slotBuildRoute(QString start_traj, QString target_traj, int dir);

private slots:

    void slotTrajChangeState(int vehicle_idx, bool is_busy, QString traj_name);
};

#endif
