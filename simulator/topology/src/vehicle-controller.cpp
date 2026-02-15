#include    "vehicle-controller.h"

#include    "Trajectory.h"
#include    "Switch.h"

#include    <physics.h>

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

    // Смещаемся на следующие траектории
    // Реализация похожа на Trajectory::findTrajectoryAtCoord(), но здесь нужно
    // сохранить и траекторную координату, и вылет за пределы траектории
    dir_t move_dir;
    while (true)
    {
        if (traj_coord < 0.0)
        {
            // Если траекторная координата меньше нуля - заехали за стрелку сзади
            move_dir = BWD;
            // Запоминаем выход за пределы траектории
            x_off = traj_coord;
        }
        else
        {
            if (traj_coord > current_traj->getLength())
            {
                // Если траекторная координата превысила длину траектории - заехали за стрелку спереди
                move_dir = FWD;
                // Запоминаем выход за пределы траектории
                x_off = traj_coord - current_traj->getLength();
            }
            else
            {
                // УРА! Остались в пределах траектории:
                // обнуляем смещение за пределы топологии и выходим
                x_off = 0.0;
                break;
            }
        }

        // Отслеживаем разворот ориентации траектории
        dir_t new_dir = move_dir;

        // Получаем указатель на стрелку в конце траектории
        Switch* next_sw = current_traj->getNextSwitch(new_dir);
        if (next_sw == nullptr)
        {
            // Если коннектора нет, останавливаемся на краю траектории и выходим
            traj_coord = traj_coord - x_off;
            break;
        }

        // Получаем указатель на ту траекторию, с которой нас соединяет стрелка
        Trajectory* next_traj = next_sw->getNextTraj(new_dir);

        // Если за стрелкой нет траектории,
        // остаёмся на исходной траектории, останавливаемся на краю и выходим
        if (next_traj == nullptr)
        {
            traj_coord = traj_coord - x_off;
            break;
        }

        // Обновляем текущую траекторию
        current_traj = next_traj;
        if (new_dir != move_dir)
        {
            // Если ориентация траектории изменилась, разворачиваемся
            dir = static_cast<dir_t>(-dir);
            x_off = -x_off;
        }

        // Смещаемся на новую траекторию, на величину смещения за пределы прежней
        if (new_dir == BWD)
        {
            // Если смещаемся назад, начинаем отсчёт с конца траектории
            traj_coord = current_traj->getLength() + x_off;
        }
        else
        {
            traj_coord = x_off;
        }
    }
/*  Старое
    // Если траекторная координата превысила длину траектории
    // (заехали за стык или стрелку спереди), пока она её превышает...
    while (traj_coord > current_traj->getLength())
    {
        // Получаем указатель на коннектор спереди
        Connector *conn = current_traj->getFwdConnector();

        // Если коннектора нет, останавливаемся на месте
        if (conn == nullptr)
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
        if (current_traj == nullptr)
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
        if (conn == nullptr)
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
        if (current_traj == nullptr)
        {
            current_traj = prev_traj;
            x_off = traj_coord;
            traj_coord = 0.0;
            break;
        }

        // Добавляем к траекторной координате длину новой траектории,
        // чтобы получить координату на новой траектории сзади
        traj_coord = current_traj->getLength() + traj_coord;
    }*/
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
void VehicleController::setDirection(int direction)
{
    (direction < 0) ? (dir = BWD) : (dir = FWD);
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
int VehicleController::getNearestVehicle(double& distance, double search_distance, dir_t direction)
{
    distance = 0.0;
    double coord = traj_coord + x_off + length_half * static_cast<double>(direction);
    Trajectory *search_traj = current_traj;

    // Проверяем, что ПЕ умещается на топологии
    if (Trajectory::findTrajectoryAtCoord(search_traj, coord, direction))
    {
        return search_traj->getBusyVehicle(distance, coord, search_distance, direction);
    }

    // Если ПЕ вылезла за какой-либо тупик, возвращаем отрицательную дистанцию,
    // чтобы затем в Train посчитать выталкивание этой ПЕ обратно
    if (coord < 0.0)
    {
        distance = coord;
    }
    else
    {
        distance = -coord;
    }
    return -1;

    // Вариант реализации в лоб, без Trajectory::findTrajectoryAtCoord()
    /*
    double move_off;
    dir_t move_dir;
    dir_t new_dir;
    while (true)
    {
        if (coord < 0.0)
        {
            // Если траекторная координата меньше нуля - заехали за стрелку сзади
            move_dir = BWD;
            // Запоминаем выход за пределы траектории
            move_off = coord;
        }
        else
        {
            if (coord > current_traj->getLength())
            {
                // Если траекторная координата превысила длину траектории - заехали за стрелку спереди
                move_dir = FWD;
                // Запоминаем выход за пределы траектории
                move_off = coord - current_traj->getLength();
            }
            else
            {
                // Остались в пределах траектории: выходим из цикла к поиску
                break;
            }
        }

        // Отслеживаем разворот ориентации траектории
        new_dir = move_dir;

        // Получаем указатель на стрелку в конце траектории
        Switch* next_sw = current_traj->getNextSwitch(new_dir);
        if (next_sw == nullptr)
        {
            // Если коннектора нет, останавливаемся на краю траектории и выходим
            distance = -(move_off * static_cast<double>(move_dir));
            return -1;
        }

        // Получаем указатель на ту траекторию, с которой нас соединяет стрелка
        Trajectory* next_traj = next_sw->getNextTraj(new_dir);

        // Если за стрелкой нет траектории,
        // остаёмся на исходной траектории, останавливаемся на краю и выходим
        if (next_traj == nullptr)
        {
            distance = -(move_off * static_cast<double>(move_dir));
            return -1;
        }

        // Обновляем текущую траекторию
        search_traj = next_traj;
        if (new_dir != move_dir)
        {
            // Если ориентация траектории изменилась, разворачиваем смещение
            move_off = -move_off;
        }

        // Смещаемся на новую траекторию, на величину смещения за пределы прежней
        if (new_dir == BWD)
        {
            // Если смещаемся назад, начинаем отсчёт с конца траектории
            coord = next_traj->getLength() + move_off;
        }
        else
        {
            coord = move_off;
        }
    }

    return search_traj->getBusyVehicle(distance, coord, search_distance, new_dir);
*/



/*  Старое
    if (direction == -1)
    {
        coord = coord - length_half;
        while (coord < 0.0)
        {
            // Получаем указатель на коннектор сзади
            Connector *conn = next_traj->getBwdConnector();
            if (conn == nullptr)
            {
                distance = coord;
                return -1;
            }

            // Получаем указатель на траекторию сзади,
            // с которой нас соединяет коннектор сзади
            next_traj = conn->getBwdTraj();
            if (next_traj == nullptr)
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
            if (conn == nullptr)
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
            if (next_traj == nullptr)
            {
                distance = -coord;
                return -1;
            }
        }

        // Поиск ближайшей ПЕ по топологии
        return next_traj->getBusyVehicle(distance, coord, search_distance, 1);
    }

    distance = search_distance;
    return -1;*/
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
/* TODO Занятость пути за пределами текущей траектории
    Trajectory* next_traj = current_traj;
    // Если траекторная координата превысила длину траектории
    // (заехали за стык или стрелку спереди), пока она её превышает...
    while (vehicle_begin > next_traj->getLength())
    {
        // Получаем указатель на коннектор спереди
        Connector *conn = next_traj->getFwdConnector();
        if (conn == nullptr)
            break;

        // Вычитаем из траекторной координаты длину предыдущей траектории,
        // чтобы получить координату на новой траектории впереди
        vehicle_begin = vehicle_begin - next_traj->getLength();

        // Получаем указатель на траекторию впереди,
        // с которой нас соединяет коннектор спереди
        next_traj = conn->getFwdTraj();
        if (next_traj == nullptr)
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
        if (conn == nullptr)
            break;

        // Получаем указатель на траекторию сзади,
        // с которой нас соединяет коннектор сзади
        next_traj = conn->getBwdTraj();
        if (next_traj == nullptr)
            break;

        // Добавляем к траекторной координате длину новой траектории,
        // чтобы получить координату на новой траектории сзади
        vehicle_end = vehicle_end + next_traj->getLength();

        // Занятость пути
        next_traj->setBusy(index, max(0.0, vehicle_end), next_traj->getLength());
    }
*/
    // Связи оборудования ПЕ с путевой инфраструктурой
    size_t i = 0;
    for (auto veh_device : *devices)
    {
        // Обновляем траекторную координату оборудования ПЕ
        veh_device.coord = traj_coord + devices_coords[i] * dir;
        ++i;

        // Текущая траектория и траекторная координата данного оборудования
        Trajectory* device_traj = current_traj;
        dir_t device_dir = dir;
        if (Trajectory::findTrajectoryAtCoord(device_traj, veh_device.coord, device_dir))
        {
            for (auto* traj_device : device_traj->getTrajectoryDevices())
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
}
