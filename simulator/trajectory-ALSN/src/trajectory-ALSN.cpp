#include    "trajectory-ALSN.h"
#include    "ALSN-coil.h"
#include    "topology-connector-device.h"
#include    "trajectory.h"
#include    "switch.h"
#include    "train-signal.h"

#include    <core/get_module.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrajectoryALSN::TrajectoryALSN(QObject *parent) : TrajectoryDevice(parent)
{
    name = QString("ALSN");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrajectoryALSN::~TrajectoryALSN()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryALSN::step(double t, double dt)
{
    (void) t;
    (void) dt;

    if (  (prev_code_from_fwd != code_from_fwd)
        ||(prev_code_from_bwd != code_from_bwd)
        ||(prev_busy_begin_coord != busy_begin_coord)
        ||(prev_busy_end_coord != busy_end_coord))
    {
        prev_code_from_fwd = code_from_fwd;
        prev_code_from_bwd = code_from_bwd;
        prev_busy_begin_coord = busy_begin_coord;
        prev_busy_end_coord = busy_end_coord;
        emit sendUpdate(serialize());
    }

    if (vehicles_devices.empty())
    {
        // Здесь делать нечего
        clear_code();
        return;
    }

    // Задаём приёмным катушкам информацию о следующем светофоре,
    // а возле начала и конца занятого участка - и код АЛСН
    size_t device_idx = 0;
    for (auto device : vehicles_devices)
    {
        std::int8_t search_dir = vehicles_devices_directions[device_idx];
        ++device_idx;

        search_dir = search_dir * device.device->getOutputSignal(CoilALSN::OUTPUT_DIRECTION);
        if (search_dir > 0)
        {
            // Литер следующего светофора
            size_t liter_size = min(static_cast<size_t>(next_liter_fwd.size()),
                                    static_cast<size_t>(CoilALSN::INPUT_LITER_MAX_SIZE));
            device.device->setInputSignal(CoilALSN::INPUT_LITER_SIZE,
                                          static_cast<double>(liter_size));
            if (liter_size > 0)
            {
                for (size_t i = 0; i < liter_size; ++i)
                {
                    device.device->setInputSignal(CoilALSN::INPUT_LITER_BEGIN + i,
                                                  static_cast<double>(next_liter_fwd.at(i).unicode()));
                }

                // Расстояние до следующего светофора, м
                device.device->setInputSignal(CoilALSN::INPUT_NEXT_DISTANCE,
                                             distance_fwd + (trajectory->getLength() - device.coord));
            }
            else
            {
                // Если следующий светофор неизвестен, неизвестно и расстояние
                device.device->setInputSignal(CoilALSN::INPUT_NEXT_DISTANCE, 0.0);
            }

            // Проверяем координату с запасом в 1 метр
            if ((busy_end_coord - device.coord) < 1.0)
            {
                // Несущая частота сигнала, Гц
                device.device->setInputSignal(CoilALSN::INPUT_FREQUENCY, frequency);
                // Кодовый сигнал
                device.device->setInputSignal(CoilALSN::INPUT_CODE, static_cast<double>(code_from_fwd));
            }
            else
            {
                // Сигнал отсутствует
                device.device->setInputSignal(CoilALSN::INPUT_FREQUENCY, 0.0);
                device.device->setInputSignal(CoilALSN::INPUT_CODE, 0.0);
            }
        }
        if (search_dir < 0)
        {
            // Литер следующего светофора
            size_t liter_size = min(static_cast<size_t>(next_liter_bwd.size()),
                                    static_cast<size_t>(CoilALSN::INPUT_LITER_MAX_SIZE));
            device.device->setInputSignal(CoilALSN::INPUT_LITER_SIZE,
                                        static_cast<double>(liter_size));
            if (liter_size > 0)
            {
                for (size_t i = 0; i < liter_size; ++i)
                {
                    device.device->setInputSignal(CoilALSN::INPUT_LITER_BEGIN + i,
                                                static_cast<double>(next_liter_bwd.at(i).unicode()));
                }

                // Расстояние до следующего светофора, м
                device.device->setInputSignal(CoilALSN::INPUT_NEXT_DISTANCE,
                                            distance_bwd + device.coord);
            }
            else
            {
                // Если следующий светофор неизвестен, неизвестно и расстояние
                device.device->setInputSignal(CoilALSN::INPUT_NEXT_DISTANCE, 0.0);
            }

            // Проверяем координату с запасом в 1 метр
            if ((busy_begin_coord - device.coord) > -1.0)
            {
                // Несущая частота сигнала, Гц
                device.device->setInputSignal(CoilALSN::INPUT_FREQUENCY, frequency);
                // Кодовый сигнал
                device.device->setInputSignal(CoilALSN::INPUT_CODE, static_cast<double>(code_from_bwd));
            }
            else
            {
                // Сигнал отсутствует
                device.device->setInputSignal(CoilALSN::INPUT_FREQUENCY, 0.0);
                device.device->setInputSignal(CoilALSN::INPUT_CODE, 0.0);
            }
        }
    }
    clear_code();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray TrajectoryALSN::serialize() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    // Кладем в буфер имя модуля
    stream << name;

    // Код спереди и расстояние действия
    stream << static_cast<std::uint8_t>(code_from_fwd);
    stream << busy_end_coord;

    // Код сзади и расстояние действия
    stream << static_cast<std::uint8_t>(code_from_bwd);
    stream << busy_begin_coord;

    return data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryALSN::deserialize(QByteArray& data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);

    // Восстанавливаем имя
    stream >> name;

    std::uint8_t tmp_ALSN;

    // Код спереди и расстояние действия
    stream >> tmp_ALSN;
    code_from_fwd = static_cast<ALSN>(tmp_ALSN);
    stream >> busy_end_coord;

    // Код сзади и расстояние действия
    stream >> tmp_ALSN;
    code_from_bwd = static_cast<ALSN>(tmp_ALSN);
    stream >> busy_begin_coord;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryALSN::getDrawElements(std::vector<draw_line_t>& lines, std::vector<draw_circle_t>& circles, const double scale)
{
    lines.reserve(trajectory->getTracks().size() * 2 + 2);

    // Вспомогательная функция для добавления параллельной треку линии в список отрисовки
    auto add_line = [](std::vector<draw_line_t>& lines, const track_t& track, double shift, int width, ALSN code)
    {
        lines.emplace_back(draw_line_t());
        draw_line_t& line = lines.back();
        line.begin_point = track.begin_point + track.trav * shift;
        line.end_point = track.end_point + track.trav * shift;
        line.width = width;

        switch (code)
        {
        case ALSN::RED_YELLOW:
        {
            line.color = {1.0f, 0.25f, 0.25f};
            break;
        }
        case ALSN::YELLOW:
        {
            line.color = {1.0f, 1.0f, 0.0f};
            break;
        }
        case ALSN::GREEN:
        {
            line.color = {0.0f, 0.75f, 0.0f};
            break;
        }
        case ALSN::NO_CODE:
        default:
        {
            line.color = {1.0f, 1.0f, 1.0f};
            break;
        }
        }
    };

    // Смещаем линию отрисовки АЛСН чуть меньше чем на целый пиксель,
    // так она будет рисоваться вдоль трека с небольшим перекрытием, чтобы не было артефактов
    double shift = 0.875;

    // Ширина отрисовки АЛСН - с учётом масштаба карты
    int width = std::ceil(2.0 * scale);

    for (const track_t& track : trajectory->getTracks())
    {
        if (track.traj_coord >= busy_end_coord)
        {
            // Траектория свободна, рисуем код АСЛН, который дошёл спереди
            add_line(lines, track, shift, width, code_from_fwd);
        }
        else if ((track.traj_coord + track.len) <= busy_end_coord)
        {
            // Траектория занята, код АСЛН спереди не дошёл сюда, рисуем белый
            add_line(lines, track, shift, width, ALSN::NO_CODE);
        }
        else
        {
            // На этом треке заканчивается занятость, код АСЛН спереди не доходит на занятую часть
            const double length_busy = busy_end_coord - track.traj_coord;
            dvec3 busy_end_point = track.begin_point + track.orth * length_busy;

            track_t busy_track;
            busy_track.begin_point = track.begin_point;
            busy_track.end_point = busy_end_point;
            busy_track.trav = track.trav;
            add_line(lines, busy_track, shift, width, ALSN::NO_CODE);

            track_t no_busy_track;
            no_busy_track.begin_point = busy_end_point;
            no_busy_track.end_point = track.end_point;
            no_busy_track.trav = track.trav;
            add_line(lines, no_busy_track, shift, width, code_from_fwd);
        }

        if (track.traj_coord > busy_begin_coord)
        {
            // Траектория занята, код АСЛН сзади не дошёл сюда, рисуем белый
            add_line(lines, track, -shift, width, ALSN::NO_CODE);
        }
        else if ((track.traj_coord + track.len) <= busy_begin_coord)
        {
            // Траектория свободна, рисуем код АСЛН, который дошёл сзади
            add_line(lines, track, -shift, width, code_from_bwd);
        }
        else
        {
            // На этом треке начинается занятость, код АСЛН сзади на этом обрывается
            const double length_no_busy = busy_begin_coord - track.traj_coord;
            dvec3 busy_begin_point = track.begin_point + track.orth * length_no_busy;

            track_t no_busy_track;
            no_busy_track.begin_point = track.begin_point;
            no_busy_track.end_point = busy_begin_point;
            no_busy_track.trav = track.trav;
            add_line(lines, no_busy_track, -shift, width, code_from_bwd);

            track_t busy_track;
            busy_track.begin_point = busy_begin_point;
            busy_track.end_point = track.end_point;
            busy_track.trav = track.trav;
            add_line(lines, busy_track, -shift, width, ALSN::NO_CODE);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryALSN::setNextSignalInfo(std::int8_t dir, ALSN code, double distance, const QString& liter)
{
    // Если на траектории нет трансмиттера с частотой, далее не передаём код,
    // только имитируем работу электронной карты - дистанцию и литер
    if (frequency == 0.0)
    {
        code = ALSN::NO_CODE;
    }

    // Вперёд рассылается код от светофора сзади, назад - от светофора спереди
    if (dir > 0)
    {
        code_from_bwd = code;
        distance_bwd = distance;
        next_liter_bwd = liter;
    }
    else
    {
        code_from_fwd = code;
        distance_fwd = distance;
        next_liter_fwd = liter;
    }

    // Проверяем, занята ли траектория
    if (trajectory->isBusy())
    {
        // Колёсные пары шунтируют рельсовые цепи и дальше код не проходит
        code = ALSN::NO_CODE;

        // Запрашиваем и сохраняем координаты занятого участка траектории
        trajectory->getBusyCoords(busy_begin_coord, busy_end_coord);
    }

    // Переход к рельсовым цепям следующей траектории
    std::int8_t next_dir = dir;
    // Модуль коннектора к следующей траектории
    auto conn_device = getNextConnectorDevice(next_dir);
    if (conn_device == nullptr)
    {
        return;
    }

    // Проверяем: если стрелка на взрез, или здесь следующий светофор,
    // дальше информацию не передаём
    Switch* conn = conn_device->getConnector();
    if (next_dir > 0)
    {
        dir_t traj_dir = BWD;
        if (conn->getNextTraj(traj_dir) != trajectory)
        {
            return;
        }
        if (dynamic_cast<TrainSignal*>(conn->getSignalBwd()))
        {
            return;
        }
    }
    else
    {
        dir_t traj_dir = FWD;
        if (conn->getNextTraj(traj_dir) != trajectory)
        {
            return;
        }
        if (dynamic_cast<TrainSignal*>(conn->getSignalFwd()))
        {
            return;
        }
    }

    // Следующая траектория
    TrajectoryALSN* traj_ALSN = dynamic_cast<TrajectoryALSN*>(
        conn_device->getNextTrajectoryDevice(next_dir));
    if (traj_ALSN == nullptr)
    {
        return;
    }

    // Передаём информацию дальше
    traj_ALSN->setNextSignalInfo(next_dir, code, distance + trajectory->getLength(), liter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryALSN::load_config(CfgReader &cfg)
{
    cfg.getDouble("ALSN", "Frequency", frequency);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryALSN::clear_code()
{
    // Очистка
    code_from_fwd = ALSN::NO_CODE;
    distance_fwd = 0.0;
    next_liter_fwd = "";
    code_from_bwd = ALSN::NO_CODE;
    distance_bwd = 0.0;
    next_liter_bwd = "";

    busy_begin_coord = trajectory->getLength();
    busy_end_coord = 0.0;
}

GET_MODULE(TrajectoryALSN)
