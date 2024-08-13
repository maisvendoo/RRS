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
void VehicleController::setRailwayCoord(double x)
{
    // Запоминаем предыдущее значение дуговой координаты
    x_prev = x_cur;
    // Обновляем значение дуговой координаты
    x_cur = x;

    // Запоминаем предыдущее значение траекторной координаты
    double prev_coord = traj_coord;

    // Обновляем траекторную координату, в соответствии с относительным
    // перемещением ПЕ
    traj_coord += dir * (x_cur - x_prev);

    // Инициализируем предыдущую траекторию как текущую
    prev_traj = current_traj;

    // Если траекторная координата превысила длину траектории (заехали за стык
    // или стрелку спереди)
    // пока она её превышает...
    while (traj_coord > current_traj->getLength())
    {
        // Получаем указатель на коннектор спереди
        Connector *conn = current_traj->getFwdConnector();

        if (conn == Q_NULLPTR)
        {
            traj_coord = prev_coord;
            return;
        }

        // Вычитаем из траекторной координаты длину текущей траектории,
        // чтобы получить координату на новой траектории впереди
        traj_coord = traj_coord - current_traj->getLength();

        // Обновляем текущую траекторию на ту, с которой нас соединяет
        // передний коннектор
        current_traj = conn->getFwdTraj();

        // если за коннектором нет траектории, возвращаемся к исходной
        // траектоиии
        if (current_traj == Q_NULLPTR)
        {
            current_traj = prev_traj;
            traj_coord = prev_coord;
            break;
        }
    }

    // Если траекторная координата меньше нуля (заехали за стык или стрелку
    // сзади)
    // пока она меньше нуля...
    while (traj_coord < 0)
    {
        // берем указатель на коннектор сзади
        Connector *conn = current_traj->getBwdConnector();

        if (conn == Q_NULLPTR)
        {
            traj_coord = prev_coord;
            return;
        }

        // теперь текущая траектория, та что за коннектором сзади
        current_traj = conn->getBwdTraj();

        if (current_traj == Q_NULLPTR)
        {
            current_traj = prev_traj;
            traj_coord = prev_coord;
            break;
        }

        // новая траекторная координата - алгебраическая сумма
        // длины новой траектории и текущей траекторной координаты
        traj_coord = current_traj->getLength() + traj_coord;
    }

    // Если перешли на новую траекторию, сбрасываем флаг занятости на
    // старой и устанавливаем его на новой
    if (current_traj != prev_traj)
    {
        prev_traj->setBusy(false);
        current_traj->setBusy(true);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setInitRailwayCoord(double x)
{
    x_prev = x_cur = x;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setInitCurrentTraj(Trajectory *traj)
{
    current_traj = prev_traj = traj;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
/*vec3d VehicleController::getPosition(vec3d &attitude) const
{
    vec3d pos = current_traj->getPosition(traj_coord, attitude);

    return pos;
}*/
