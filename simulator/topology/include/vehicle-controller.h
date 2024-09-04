#ifndef     VEHICLE_CONTROLLER_H
#define     VEHICLE_CONTROLLER_H

#include    <QObject>

#include    <topology-types.h>
#include    <topology-export.h>
#include    <profile-point.h>
#include    <device-list.h>
#include    <device-joint.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT VehicleController : public QObject
{
    Q_OBJECT

public:

    VehicleController(QObject *parent = Q_NULLPTR);

    virtual ~VehicleController();

    /// Устанавливаем индекс данной ПЕ в симуляции
    void setIndex(size_t idx);

    /// Устанавливаем длину данной ПЕ
    void setLength(double len);

    /// Устанавливаем текущую дуговую координату ПЕ
    void setRailwayCoord(double x);

    /// Задание начальной дуговой координаты ПЕ
    void setInitRailwayCoord(double x);

    /// Задание начальной траекториии и положения ПЕ на ней
    void setInitCurrentTraj(Trajectory *traj, double traj_coord);

    /// Устанавливаем направление движения по траектории
    void setDirection(int dir) { this->dir = dir; }

    /// Задать оборудование ПЕ, взаимодействующее с путевой инфраструктурой
    void setVehicleRailwayConnectors(device_coord_list_t *devices);

    /// Указатели на оборудование ПЕ, взаимодействующее с путевой инфраструктурой
    device_coord_list_t *getVehicleRailwayConnectors();

    /// Вернуть структуру, определяющую положение ПЕ в пространстве
    profile_point_t getPosition(int dir);

    /// Вернуть массив указателей на траектории, занятые ПЕ
    Trajectory *getCurrentTraj() const { return current_traj; }

    /// Вернуть текущую координату вдоль траектории
    double getTrajCoord() const { return traj_coord; }

    /// Шаг симуляции
    void step(double t, double dt);

protected:

    /// Индекс данной ПЕ в симуляции
    size_t index = 0;

    /// Половина длины данной ПЕ
    double length_half = 10.0;

    /// Текущее значение дуговой координаты ПЕ
    double  x_cur = 0.0;

    /// Направление движения
    int    dir = 1;

    /// Координата, в пределах текущей траектории
    double traj_coord = 0.0;

    /// Текущая траектория ПЕ
    Trajectory *current_traj = Q_NULLPTR;

    /// Предыдущая траектория ПЕ (за коннектором сзади по ходу движения)
    Trajectory *prev_traj = Q_NULLPTR;

    /// Оборудование ПЕ, взаимодействующее с путевой инфраструктурой,
    /// и его текущие траекторные координаты
    device_coord_list_t *devices = {};

    /// Смещения вперёд-назад оборудования ПЕ
    std::vector<double> devices_coords;

    /// Расчёт занятости траекторий по всей длине ПЕ
    /// и связей оборудования ПЕ с путевой инфраструктурой
    void updateTrajectories();
};

#endif // VEHICLE_CONTROLLER_H
