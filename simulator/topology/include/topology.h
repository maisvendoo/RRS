#ifndef     TOPOLOGY_H
#define     TOPOLOGY_H

#include    <QObject>
#include    <unordered_map>
#include    <vector>
#include    <string>
#include    <utility>

#include    <topology-export.h>
#include    <topology-types.h>
#include    <trajectory.h>
#include    <signals-data-types.h>
#include    <route-command.h>
#include    <route-segment.h>

class Vehicle;
class VehicleController;

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
    VehicleController& getVehicleController(size_t idx);

    /// Нахождение пути в графе траекторий
    route_segment_t find_route(Trajectory *start_traj,
                               Trajectory *target_traj,
                               qint8 dir,
                               bool check_busy = true);

    /// Получить распрямлённый профиль пути вокруг точки (traj, coord):
    /// траектории вперёд и назад от точки на backward_m/forward_m метров
    bool getProfile(Trajectory *traj, double coord, dir_t orient,
                    double backward_m, double forward_m,
                    profile_segments_t &out) const;

    /// Шаг симуляции
    void step(double t, double dt);

    QByteArray serialize() const;
    QByteArray serialize_modules() const;
    QByteArray serialize_stations() const;

    void deserialize(QByteArray& data);
    void deserialize_modules(QByteArray& data);

    traj_list_t *getTrajectoriesList();
    const traj_list_t* getTrajectoriesList() const;

    sw_list_t *getConnectorsList();
    const sw_list_t* getConnectorsList() const;

    topology_stations_list_t *getStationsList();

    signals_data_t *getSignalsData();

    QString getRouteName() const;

signals:

    void sendSwitchState(QByteArray sw_data);

    void sendTrajBusyState(QByteArray busy_data);

    void sendModuleUpdate(QByteArray module_data);

    void sigSetOpenSignalsQueue(std::vector<std::pair<QString, int>> conn_list, bool for_train, bool for_shunting);

    void sigChangeTrajStateByTrain(int train_idx, bool is_busy, QString traj_name);

    void sigIncTargetStation(int vehicle_idx, bool is_on_target_traj);

    void sigCalcMiddleVelocity(int vehicle_idx, double target_dist);

    void sigGetTrajState(int vehicle_idx,
                         int station_idx,
                         QString start_traj_name,
                         QString traj_name,
                         int request_type,
                         bool is_route_possible);

private:

    /// Контейнер данных по всем траекториям на полигоне
    traj_list_t traj_list;

    /// Контейнер стрелок
    sw_list_t   switches;

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
    void load_signals(CfgReader &cfg, QDomNode secNode, Switch* sw);

    /// Загрузка списка станций
    bool load_stations(QString route_dir);

    /// Получение название маршрута из конфига описания
    void get_route_name(QString route_dir);

    void serialize_connector_name(QDataStream &stream, const Switch* sw) const;

    Switch* deserialize_traj_connectors(QDataStream &stream, sw_list_t &conn_list) const;

    /// Команда построения маршрута
    route_segment_t build_route(const route_command_t& rc);

    /// Установка стрелок по маршруту
    bool set_switchs_by_route(const route_segment_t& route);

    /// Октрытие попутных сигналов по маршруту
    bool open_route_signals(const route_segment_t& route, std::vector<std::pair<QString, int>>& sw_list, bool for_train = true);

public slots:

    void slotGetSwitchState(QByteArray &switch_data);

    void slotSwitchCommand(QByteArray& switch_data);

    void slotSignalCommand(QByteArray& signal_data);

    void slotBuildRouteCommand(QByteArray& route_data);

    void slotTrainRouteCommand(QByteArray& route_data);

    void slotShuntingRouteCommand(QByteArray& route_data);

    void slotGetTrajState(QString traj_name, bool &is_busy, bool &in_route);

    void slotGetNextTrajName(QString traj_name, int dir, QString &next_traj_name);

    void slotIsRouteExists(QString start_traj_name, QString end_traj_name, int dir, bool *exists);

    void slotGetRouteLength(int vehicle_idx, QString cur_traj_name,
                            double cur_coord,
                            QString target_traj_name,
                            double target_coord,
                            int dir,
                            double *lenght);

    void slotGetTrajStateRequest(int vehicle_idx, int station_idx, QString start_traj_name, QString traj_name, int dir, int request_type);

    void slotTrajModuleUpdate(QByteArray& traj_module_data);

private slots:

    void slotTrajChangeState(int vehicle_idx, bool is_busy, QString traj_name);
};

#endif
