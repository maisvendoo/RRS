#include    <vehicle-controller.h>

#include    <connector.h>

#include    "physics.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleController::VehicleController(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleController::~VehicleController()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setIndex(size_t idx)
{
    index = idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setLength(double len)
{
    if (len > Physics::ZERO)
        length_half = len / 2.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setCoord(double x)
{
    // Обновляем траекторную координату,
    // в соответствии с относительным перемещением ПЕ
    traj_coord += x - x_cur + x_off;
    // Обновляем значение дуговой координаты
    x_cur = x;
    // Обнуляем выход за тупики топологии
    x_off = 0.0;

    // Инициализируем предыдущую траекторию как текущую
    prev_traj = current_traj;

    // Если траекторная координата превысила длину траектории
    // (заехали за стык или стрелку спереди), пока она её превышает...
    while (traj_coord > current_traj->getLength())
    {
        // Получаем указатель на коннектор спереди
        Connector *conn = current_traj->getFwdConnector();

        // Если коннектора нет, останавливаемся на месте
        if (conn == Q_NULLPTR)
        {
            x_off = traj_coord - current_traj->getLength();
            traj_coord = current_traj->getLength();
            return;
        }

        // Обновляем текущую траекторию на ту,
        // с которой нас соединяет коннектор спереди
        current_traj = conn->getFwdTraj();

        // Если за коннектором нет траектории,
        // возвращаемся к исходной траектории и останавливаемся на месте
        if (current_traj == Q_NULLPTR)
        {
            current_traj = prev_traj;
            x_off = traj_coord - current_traj->getLength();
            traj_coord = current_traj->getLength();
            break;
        }

        // Вычитаем из траекторной координаты длину предыдущей траектории,
        // чтобы получить координату на новой траектории впереди
        traj_coord = traj_coord - prev_traj->getLength();
    }

    // Если траекторная координата меньше нуля
    // (заехали за стык или стрелку сзади), пока она меньше нуля...
    while (traj_coord < 0.0)
    {
        // Получаем указатель на коннектор сзади
        Connector *conn = current_traj->getBwdConnector();

        // Если коннектора нет, останавливаемся на месте
        if (conn == Q_NULLPTR)
        {
            x_off = traj_coord;
            traj_coord = 0.0;
            return;
        }

        // Обновляем текущую траекторию на ту,
        // с которой нас соединяет коннектор сзади
        current_traj = conn->getBwdTraj();

        // Если за коннектором нет траектории,
        // возвращаемся к исходной траектории и останавливаемся на месте
        if (current_traj == Q_NULLPTR)
        {
            current_traj = prev_traj;
            x_off = traj_coord;
            traj_coord = 0.0;
            break;
        }

        // Добавляем к траекторной координате длину новой траектории,
        // чтобы получить координату на новой траектории сзади
        traj_coord = current_traj->getLength() + traj_coord;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setInitCoord(double x)
{
    x_cur = x;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setInitCurrentTraj(Trajectory *traj, double traj_coord)
{
    this->current_traj = traj;
    this->traj_coord = traj_coord;

    updateTrajectories();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setVehicleRailwayConnectors(device_coord_list_t *devices)
{
    this->devices = devices;

    // Сохраняем смещения вперёд-назад в отдельном массиве,
    // чтобы в структуре device_coord_t хранить траекторные координаты
    for (auto veh_device : *devices)
    {
        devices_coords.push_back(veh_device.coord);
        veh_device.coord = 0.0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
device_coord_list_t *VehicleController::getVehicleRailwayConnectors()
{
    return devices;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
profile_point_t VehicleController::getPosition()
{
    return current_traj->getPosition(traj_coord, dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int VehicleController::getNearestVehicle(double &distance, double search_distance, int direction)
{
    distance = 0.0;
    double coord = traj_coord + x_off;
    Trajectory *next_traj = current_traj;
    if (direction == -1)
    {
        coord = coord - length_half;
        while (coord < 0.0)
        {
            // Получаем указатель на коннектор сзади
            Connector *conn = next_traj->getBwdConnector();
            if (conn == Q_NULLPTR)
            {
                distance = coord;
                return -1;
            }

            // Получаем указатель на траекторию сзади,
            // с которой нас соединяет коннектор сзади
            next_traj = conn->getBwdTraj();
            if (next_traj == Q_NULLPTR)
            {
                distance = coord;
                return -1;
            }

            // Добавляем к траекторной координате длину новой траектории,
            // чтобы получить координату на новой траектории сзади
            coord = coord + next_traj->getLength();
        }

        // Поиск ближайшей ПЕ по топологии
        return next_traj->getBusyVehicle(distance, coord, search_distance, -1);
    }
    else
    {
        coord = coord + length_half;
        while (coord > next_traj->getLength())
        {
            // Получаем указатель на коннектор спереди
            Connector *conn = next_traj->getFwdConnector();
            if (conn == Q_NULLPTR)
            {
                distance = next_traj->getLength() - coord;
                return -1;
            }

            // Вычитаем из траекторной координаты длину предыдущей траектории,
            // чтобы получить координату на новой траектории впереди
            coord = coord - next_traj->getLength();

            // Получаем указатель на траекторию впереди,
            // с которой нас соединяет коннектор спереди
            next_traj = conn->getFwdTraj();
            if (next_traj == Q_NULLPTR)
            {
                distance = -coord;
                return -1;
            }
        }

        // Поиск ближайшей ПЕ по топологии
        return next_traj->getBusyVehicle(distance, coord, search_distance, 1);
    }

    distance = search_distance;
    return -1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::step(double t, double dt)
{
    (void) t;
    (void) dt;
    updateTrajectories();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::updateTrajectories()
{
    double vehicle_begin = traj_coord + x_off + length_half;
    double vehicle_end = traj_coord + x_off - length_half;

    // Занятость пути
    current_traj->setBusy(index, max(0.0, vehicle_end), min(vehicle_begin, current_traj->getLength()));

    Trajectory *next_traj = current_traj;
    // Если траекторная координата превысила длину траектории
    // (заехали за стык или стрелку спереди), пока она её превышает...
    while (vehicle_begin > next_traj->getLength())
    {
        // Получаем указатель на коннектор спереди
        Connector *conn = next_traj->getFwdConnector();
        if (conn == Q_NULLPTR)
            break;

        // Вычитаем из траекторной координаты длину предыдущей траектории,
        // чтобы получить координату на новой траектории впереди
        vehicle_begin = vehicle_begin - next_traj->getLength();

        // Получаем указатель на траекторию впереди,
        // с которой нас соединяет коннектор спереди
        next_traj = conn->getFwdTraj();
        if (next_traj == Q_NULLPTR)
            break;

        // Занятость пути
        next_traj->setBusy(index, 0.0, min(vehicle_begin, next_traj->getLength()));
    }

    next_traj = current_traj;
    // Если траекторная координата меньше нуля
    // (заехали за стык или стрелку сзади), пока она меньше нуля...
    while (vehicle_end < 0.0)
    {
        // Получаем указатель на коннектор сзади
        Connector *conn = next_traj->getBwdConnector();
        if (conn == Q_NULLPTR)
            break;

        // Получаем указатель на траекторию сзади,
        // с которой нас соединяет коннектор сзади
        next_traj = conn->getBwdTraj();
        if (next_traj == Q_NULLPTR)
            break;

        // Добавляем к траекторной координате длину новой траектории,
        // чтобы получить координату на новой траектории сзади
        vehicle_end = vehicle_end + next_traj->getLength();

        // Занятость пути
        next_traj->setBusy(index, max(0.0, vehicle_end), next_traj->getLength());
    }

    // Связи оборудования ПЕ с путевой инфраструктурой
    size_t i = 0;
    for (auto veh_device : *devices)
    {
        // Обновляем траекторную координату оборудования ПЕ
        veh_device.coord = traj_coord + devices_coords[i] * dir;
        ++i;

        // Текущая траектория и траекторная координата данного оборудования
        next_traj = current_traj;
        while (veh_device.coord > next_traj->getLength())
        {
            Connector *conn = next_traj->getFwdConnector();
            if (conn == Q_NULLPTR)
            {
                next_traj = Q_NULLPTR;
                break;
            }

            veh_device.coord = veh_device.coord - next_traj->getLength();

            next_traj = conn->getFwdTraj();
            if (next_traj == Q_NULLPTR)
            {
                break;
            }
        }

        while (veh_device.coord < 0.0)
        {
            Connector *conn = next_traj->getBwdConnector();
            if (conn == Q_NULLPTR)
            {
                next_traj = Q_NULLPTR;
                break;
            }

            next_traj = conn->getBwdTraj();
            if (next_traj == Q_NULLPTR)
            {
                break;
            }

            veh_device.coord = veh_device.coord + next_traj->getLength();
        }

        if (next_traj == Q_NULLPTR)
            continue;

        for (auto traj_device : next_traj->getTrajectoryDevices())
        {
            // Связываем оборудование ПЕ и путевое оборудование
            if (veh_device.device->getName() == traj_device->getName())
            {
                traj_device->setLink(veh_device);
                break;
            }
        }
    }
}
