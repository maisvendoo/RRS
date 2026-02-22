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
size_t VehicleController::getIndex() const
{
    return index;
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
void VehicleController::setPathCoord(double x)
{
    // Обновляем траекторную координату,
    // в соответствии с относительным перемещением ПЕ
    traj_coord += static_cast<double>(orientation) * (x - x_cur) + x_off;
    // Обновляем значение дуговой координаты
    x_cur = x;

    // Смещаемся на следующие траектории
    Trajectory::findTrajectoryAtCoord(current_traj, traj_coord, x_off, orientation);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setInitPathCoord(double x)
{
    x_cur = x;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setInitCurrentTraj(Trajectory *traj, double coord, dir_t direction)
{
    current_traj = traj;
    traj_coord = coord;
    (direction < 0) ? (orientation = BWD) : (orientation = FWD);

    updateTrajectories();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
dir_t VehicleController::getOrientation() const
{
    return orientation;
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
    return current_traj->getPosition(traj_coord, orientation);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int VehicleController::getNearestVehicle(double& distance, double search_distance, dir_t direction)
{
    distance = 0.0;
    direction = static_cast<dir_t>(orientation * direction);
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

    // Занятость пути на соседних траекториях на длину ПЕ
    // Реализация похожа на Trajectory::findTrajectoryAtCoord(), но здесь нужно
    // вызвать Trajectory::setBusy для каждой посещённой траектории
    auto set_busy_off = [](size_t index, Trajectory* cur_traj, double coord)
    {
        dir_t move_dir;
        while (true)
        {
            if (coord < 0.0)
            {
                // Если траекторная координата меньше нуля - заехали за стрелку сзади
                move_dir = BWD;
            }
            else
            {
                if (coord > cur_traj->getLength())
                {
                    // Если траекторная координата превысила длину траектории - заехали за стрелку спереди
                    move_dir = FWD;
                    // Учитываем выход за пределы траектории
                    coord = coord - cur_traj->getLength();
                }
                else
                {
                    // УРА! Находимся в пределах траектории: выходим
                    return;
                }
            }

            // Отслеживаем разворот ориентации траектории
            dir_t new_dir = move_dir;

            // Получаем указатель на стрелку в конце траектории
            Switch* next_sw = cur_traj->getNextSwitch(new_dir);
            if (next_sw == nullptr)
            {
                // Если коннектора нет, выходим
                return;
            }

            // Получаем указатель на ту траекторию, с которой нас соединяет стрелка
            Trajectory* next_traj = next_sw->getNextTraj(new_dir);

            // Если за стрелкой нет траектории, выходим
            if (next_traj == nullptr)
            {
                return;
            }

            // Обновляем текущую траекторию
            cur_traj = next_traj;
            if (new_dir != move_dir)
            {
                // Если ориентация траектории изменилась, разворачиваемся
                coord = -coord;
            }

            if (new_dir == BWD)
            {
                // Если смещаемся назад, начинаем отсчёт с конца траектории
                coord = coord + cur_traj->getLength();

                // Занятость пути
                cur_traj->setBusy(index, max(0.0, coord), cur_traj->getLength());
            }
            else
            {
                // Занятость пути
                cur_traj->setBusy(index, 0.0, min(coord, cur_traj->getLength()));
            }
        }
    };

    set_busy_off(index, current_traj, vehicle_begin);
    set_busy_off(index, current_traj, vehicle_end);


    // Связи оборудования ПЕ с путевой инфраструктурой
    size_t i = 0;
    for (auto veh_device : *devices)
    {
        // Обновляем траекторную координату оборудования ПЕ
        veh_device.coord = traj_coord + devices_coords[i] * orientation;
        ++i;

        // Текущая траектория и траекторная координата данного оборудования
        Trajectory* device_traj = current_traj;
        dir_t device_dir = orientation;
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
