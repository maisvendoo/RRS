#ifndef     TOPOLOGY_H
#define     TOPOLOGY_H

#include    <QObject>

#include    <topology-export.h>
#include    <topology-types.h>
#include    <vehicle-controller.h>
#include    <vehicle.h>
#include    <line-signal.h>

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

    Topology(QObject *parent = Q_NULLPTR);

    ~Topology();

    /// Загрузка топологии ж/д полигона
    bool load(QString route_dir);

    /// Общая инициализация
    bool init(const topology_pos_t &tp, std::vector<Vehicle *> *vehicles);

    /// Вернуть контроллер конкретной ПЕ
    VehicleController *getVehicleController(size_t idx);

    /// Шаг симуляции
    void step(double t, double dt);

    QByteArray serialize();

    void deserialize(QByteArray &data);

    traj_list_t *getTrajectoriesList()
    {
        return &traj_list;
    }

    conn_list_t *getConnectorsList()
    {
        return &switches;
    }

    topology_stations_list_t *getStationsList()
    {
        return &stations;
    }

    QString getRouteName() const
    {
        return route_name;
    }

signals:

    void sendSwitchState(QByteArray sw_data);

    void sendTrajBusyState(QByteArray busy_data);

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

    /// Контейнер проходных сигналов
    std::vector<Signal *> line_signals;

    /// Название маршрута
    QString route_name = "";

    /// Получить список имен всех имеющихся траекторий
    QStringList getTrajNamesList(QString route_dir);

    /// Загрузка топологии
    bool load_topology(QString route_dir);

    /// Загрузка сигналов (пока ограничиваюсь проходными)
    void load_signals(CfgReader &cfg, QDomNode secNode, Connector *conn);

    /// Связывание сигналов
    void line_signals_connect(std::vector<Signal *> &line_signals);

    /// Загрузка списка станций
    bool load_stations(QString route_dir);

    /// Получение название маршрута из конфига описания
    void get_route_name(QString route_dir);

    void serialize_connector_name(QDataStream &stream, Connector *conn);

    Connector *deserialize_traj_connectors(QDataStream &stream, conn_list_t &conn_list) const;

public slots:

    void getSwitchState(QByteArray &switch_data);
};

#endif
