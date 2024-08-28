#include    <vehicle-controller.h>

#include    <connector.h>

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
void VehicleController::setRailwayCoord(double x)
{
    // Запоминаем предыдущее значение траекторной координаты
    double prev_coord = traj_coord;

    // Обновляем траекторную координату,
    // в соответствии с относительным перемещением ПЕ
    traj_coord += x - x_cur;
    // Обновляем значение дуговой координаты
    x_cur = x;

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
            traj_coord = prev_coord;
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
            traj_coord = prev_coord;
            break;
        }

        // Убираем занятость предыдущей траектории, задав отрицательную координату
        prev_traj->setBusy(index, -1.0);

        // Вычитаем из траекторной координаты длину предыдущей траектории,
        // чтобы получить координату на новой траектории впереди
        traj_coord = traj_coord - prev_traj->getLength();
    }

    // Если траекторная координата меньше нуля
    // (заехали за стык или стрелку сзади), пока она меньше нуля...
    while (traj_coord < 0)
    {
        // Получаем указатель на коннектор сзади
        Connector *conn = current_traj->getBwdConnector();

        // Если коннектора нет, останавливаемся на месте
        if (conn == Q_NULLPTR)
        {
            traj_coord = prev_coord;
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
            traj_coord = prev_coord;
            break;
        }

        // Убираем занятость предыдущей траектории, задав отрицательную координату
        prev_traj->setBusy(index, -1.0);

        // новая траекторная координата - алгебраическая сумма
        // длины новой траектории и текущей траекторной координаты
        traj_coord = current_traj->getLength() + traj_coord;
    }

    // Обновляем занятость текущей траектории
    current_traj->setBusy(index, traj_coord);

    // Если перешли на новую траекторию, перебрасываем и модули тоже
    if (current_traj != prev_traj)
    {
        for (auto traj_device : prev_traj->getTrajectoryDevices())
        {
            traj_device->setLink(nullptr, index);
        }

        for (auto veh_device : *devices)
        {
            for (auto traj_device : current_traj->getTrajectoryDevices())
            {
                if (veh_device->getName() == traj_device->getName())
                    traj_device->setLink(veh_device, index);
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setVehicleRailwayConnectors(device_list_t *devices)
{
    this->devices = devices;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
device_list_t *VehicleController::getVehicleRailwayConnectors()
{
    return devices;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setInitRailwayCoord(double x)
{
    x_cur = x;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setInitCurrentTraj(Trajectory *traj)
{
    current_traj = prev_traj = traj;
    for (auto veh_device : *devices)
    {
        for (auto traj_device : current_traj->getTrajectoryDevices())
        {
            if (veh_device->getName() == traj_device->getName())
                traj_device->setLink(veh_device, index);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
profile_point_t VehicleController::getPosition(int dir)
{
    return current_traj->getPosition(traj_coord, dir);
}
