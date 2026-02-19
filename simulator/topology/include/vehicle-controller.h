#ifndef     VEHICLE_CONTROLLER_H
#define     VEHICLE_CONTROLLER_H

#include    <QObject>

#include    <topology-defines.h>
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

    VehicleController(QObject *parent = nullptr);

    virtual ~VehicleController();

    /// Устанавливаем индекс данной ПЕ в симуляции
    void setIndex(size_t idx);

    /// Индекс данной ПЕ в симуляции
    size_t getIndex() const;

    /// Устанавливаем длину данной ПЕ
    void setLength(double len);

    /// Устанавливаем текущую дуговую координату ПЕ
    void setPathCoord(double x);

    /// Задание начальной дуговой координаты ПЕ
    void setInitPathCoord(double x);

    /// Задание начальной траекториии и положения ПЕ на ней
    void setInitCurrentTraj(Trajectory *traj, double coord, dir_t direction);

    /// Ориентация на траектории
    dir_t getOrientation() const;

    /// Задать оборудование ПЕ, взаимодействующее с путевой инфраструктурой
    void setVehicleRailwayConnectors(device_coord_list_t *devices);

    /// Указатели на оборудование ПЕ, взаимодействующее с путевой инфраструктурой
    device_coord_list_t *getVehicleRailwayConnectors();

    /// Вернуть структуру, определяющую положение ПЕ в пространстве
    profile_point_t getPosition();

    /// Индекс ближайшей единицы подвижного состава, если есть;
    /// -1, если нет подвижного состава в пределах дистанции поиска
    int getNearestVehicle(double& distance, double search_distance, dir_t direction);

    /// Шаг симуляции
    void step(double t, double dt);

    void setTrainIndex(size_t train_idx)
    {
        this->train_idx = train_idx;
    }

    size_t getTrainIndex() const
    {
        return train_idx;
    }

private:

    /// Индекс данной ПЕ в симуляции
    size_t index = 0;

    /// Индекс поезда, которому данный контроллер соответствует
    size_t train_idx = 0;

    /// Половина длины данной ПЕ
    double length_half = 10.0;

    /// Текущее значение дуговой координаты ПЕ
    double x_cur = 0.0;

    /// Выход дуговой координаты ПЕ за тупик топологии
    double x_off = 0.0;

    /// Координата, в пределах текущей траектории
    double traj_coord = 0.0;

    /// Направление движения
    dir_t orientation = FWD;

    /// Текущая траектория ПЕ
    Trajectory *current_traj = nullptr;

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
