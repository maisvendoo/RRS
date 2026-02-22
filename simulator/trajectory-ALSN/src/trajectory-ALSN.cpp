#include    "trajectory-ALSN.h"
#include    "ALSN-coil.h"
#include    "topology-connector-device.h"
#include    "trajectory.h"
#include    "switch.h"
#include    "train-signal.h"

#include    "physics.h"

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

    if (vehicles_devices.empty())
    {
        // Здесь делать нечего, в конце выполнения шага очищаем информацию об АЛСН
        clear_code();
        return;
    }

    // Координаты занятого участка траектории
    // (от начала первой ПЕ до конца последней ПЕ);
    // между ними сигнала АЛСН нет,
    // так как он зашунтирован колёсными парами ПЕ
    double busy_begin_coord;
    double busy_end_coord;
    trajectory->getBusyCoords(busy_begin_coord, busy_end_coord);

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

    // В конце выполнения шага очищаем информацию об АЛСН
    clear_code();
    return;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryALSN::setNextSignalInfo(std::int8_t dir, ALSN code, double distance, QString liter)
{
    // Вперёд рассылается код от светофора сзади, назад - от светофора спереди
    ALSN& code_from_dir = (dir > 0) ? code_from_bwd : code_from_fwd;
    double& distance_dir = (dir > 0) ? distance_bwd : distance_fwd;
    QString& next_liter_dir = (dir > 0) ? next_liter_bwd : next_liter_fwd;

    if (frequency == 0.0)
    {
        code_from_dir = ALSN::NO_CODE;
    }
    else
    {
        code_from_dir = code;
    }

    distance_dir = distance;
    next_liter_dir = liter;

    ALSN code_to_next = code_from_dir;

    // Если траектория занята, дальше код не проходит
    if (trajectory->isBusy())
        code_to_next = ALSN::NO_CODE;

    // Переход к рельсовым цепям следующей траектории
    std::int8_t next_dir = dir;
    // Модуль коннектора к следующей траектории
    auto conn_device = getNextConnectorDevice(dir);
    if (conn_device == nullptr)
    {
        return;
    }

    // Проверяем: если стрелка на взрез, или здесь следующий светофор, дальше код не проходит
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
    traj_ALSN->setNextSignalInfo(next_dir, code_to_next, distance + trajectory->getLength(), liter);
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
}

GET_TRAJECTORY_DEVICE(TrajectoryALSN)
