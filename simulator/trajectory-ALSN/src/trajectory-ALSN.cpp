#include    "trajectory-ALSN.h"
#include    "ALSN-coil.h"
#include    "topology-connector-device.h"
#include    "trajectory.h"

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
        return;

    if (frequency == 0.0)
    {
        for (auto device : vehicles_devices)
        {
            // Сигнал отсутствует
            device.device->setInputSignal(CoilALSN::INPUT_FREQUENCY, 0.0);
            device.device->setInputSignal(CoilALSN::INPUT_CODE, 0.0);
            device.device->setInputSignal(CoilALSN::INPUT_NEXT_DISTANCE, 0.0);
            device.device->setInputSignal(CoilALSN::INPUT_LITER_SIZE, 0.0);
        }
        return;
    }

    // Координаты занятого участка траектории
    // (от начала первой ПЕ до конца последней ПЕ);
    // между ними сигнала АЛСН нет,
    // так как он зашунтирован колёсными парами ПЕ
    double busy_begin_coord;
    double busy_end_coord;
    trajectory->getBusyCoords(busy_begin_coord, busy_end_coord);

    // Ищем приёмные катушки АЛСН возле начала и конца занятого участка
    Device *first_device = nullptr;
    double first_coord;
    Device *last_device = nullptr;
    double last_coord;
    for (auto device : vehicles_devices)
    {
        if (device.device->getOutputSignal(CoilALSN::OUTPUT_DIRECTION) == 1.0)
        {
            // Проверяем координату с запасом в 1 метр
            if ((busy_begin_coord - device.coord) < 1.0)
            {
                first_coord = device.coord;
                first_device = device.device;
            }
            else
            {
                // Сигнал отсутствует
                device.device->setInputSignal(CoilALSN::INPUT_FREQUENCY, 0.0);
                device.device->setInputSignal(CoilALSN::INPUT_CODE, 0.0);
                device.device->setInputSignal(CoilALSN::INPUT_NEXT_DISTANCE, 0.0);
                device.device->setInputSignal(CoilALSN::INPUT_LITER_SIZE, 0.0);
            }
        }
        if (device.device->getOutputSignal(CoilALSN::OUTPUT_DIRECTION) == -1.0)
        {
            // Проверяем координату с запасом в 1 метр
            if ((busy_end_coord - device.coord) > -1.0)
            {
                last_coord = device.coord;
                last_device = device.device;
            }
            else
            {
                // Сигнал отсутствует
                device.device->setInputSignal(CoilALSN::INPUT_FREQUENCY, 0.0);
                device.device->setInputSignal(CoilALSN::INPUT_CODE, 0.0);
                device.device->setInputSignal(CoilALSN::INPUT_NEXT_DISTANCE, 0.0);
                device.device->setInputSignal(CoilALSN::INPUT_LITER_SIZE, 0.0);
            }
        }
    }

    if (first_device != nullptr)
    {
        // Несущая частота сигнала, Гц
        first_device->setInputSignal(CoilALSN::INPUT_FREQUENCY, frequency);
        // Кодовый сигнал
        first_device->setInputSignal(CoilALSN::INPUT_CODE, static_cast<double>(code_from_fwd));

        // Расстояние до следующего светофора, м
        first_device->setInputSignal(CoilALSN::INPUT_NEXT_DISTANCE,
            distance_fwd + (trajectory->getLength() - first_coord));

        // Литер следующего светофора
        size_t liter_size = min(static_cast<size_t>(next_liter_fwd.size()),
                                static_cast<size_t>(CoilALSN::INPUT_LITER_MAX_SIZE));
        first_device->setInputSignal(CoilALSN::INPUT_LITER_SIZE,
                                     static_cast<double>(liter_size));
        if (liter_size > 0)
        {
            for (size_t i = 0; i < liter_size; ++i)
            {
                first_device->setInputSignal(CoilALSN::INPUT_LITER_BEGIN + i,
                                             static_cast<double>(next_liter_fwd.at(i).unicode()));
            }
        }
    }

    if (last_device != nullptr)
    {
        // Несущая частота сигнала, Гц
        last_device->setInputSignal(CoilALSN::INPUT_FREQUENCY, frequency);
        // Кодовый сигнал
        last_device->setInputSignal(CoilALSN::INPUT_CODE, static_cast<double>(code_from_bwd));

        // Расстояние до следующего светофора, м
        last_device->setInputSignal(CoilALSN::INPUT_NEXT_DISTANCE,
                                     distance_bwd + last_coord);

        // Литер следующего светофора
        size_t liter_size = min(static_cast<size_t>(next_liter_bwd.size()),
                                static_cast<size_t>(CoilALSN::INPUT_LITER_MAX_SIZE));
        last_device->setInputSignal(CoilALSN::INPUT_LITER_SIZE,
                                     static_cast<double>(liter_size));
        if (liter_size > 0)
        {
            for (size_t i = 0; i < liter_size; ++i)
            {
                last_device->setInputSignal(CoilALSN::INPUT_LITER_BEGIN + i,
                                             static_cast<double>(next_liter_bwd.at(i).unicode()));
            }
        }
    }

    // Очистка
    code_from_fwd = ALSN::NO_CODE;
    distance_fwd = 0.0;
    next_liter_fwd = "";
    code_from_bwd = ALSN::NO_CODE;
    distance_bwd = 0.0;
    next_liter_bwd = "";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryALSN::setSignalInfoFwd(ALSN code, double distance, QString liter)
{
    if (frequency == 0.0)
    {
        return;
    }

    code_from_fwd = code;
    distance_fwd = distance;
    next_liter_fwd = liter;

    ALSN code_to_next = code;

    // Переход к рельсовым цепям предыдущей траектории
    // Модуль коннектора к предыдущей траектории
    auto conn_device = getBwdConnectorDevice();
    if (conn_device == nullptr)
        return;

    // Проверяем что коннектор не установлен на взрез стрелки
    TrajectoryDevice *traj_device = conn_device->getFwdTrajectoryDevice();
    if (traj_device != this)
    {
        // Если взрез, сбрасываем код
        ALSN code_to_next = ALSN::NO_CODE;
    }

    // Предыдущая траектория
    TrajectoryALSN *traj_ALSN = dynamic_cast<TrajectoryALSN *>(
        conn_device->getBwdTrajectoryDevice());
    if (traj_ALSN == nullptr)
        return;

    // Передаём информацию дальше
    traj_ALSN->setSignalInfoFwd(code_to_next, distance + trajectory->getLength(), liter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryALSN::setSignalInfoBwd(ALSN code, double distance, QString liter)
{
    if (frequency == 0.0)
    {
        return;
    }

    code_from_bwd = code;
    distance_bwd = distance;
    next_liter_bwd = liter;

    ALSN code_to_next = code;

    // Переход к рельсовым цепям следующей траектории
    // Модуль коннектора к следующей траектории
    auto conn_device = getFwdConnectorDevice();
    if (conn_device == nullptr)
        return;

    // Проверяем что коннектор не установлен на взрез стрелки
    TrajectoryDevice *traj_device = conn_device->getBwdTrajectoryDevice();
    if (traj_device != this)
    {
        // Если взрез, сбрасываем код
        ALSN code_to_next = ALSN::NO_CODE;
    }

    // Следующая траектория
    TrajectoryALSN *traj_ALSN = dynamic_cast<TrajectoryALSN *>(
        conn_device->getFwdTrajectoryDevice());
    if (traj_ALSN == nullptr)
        return;

    // Передаём информацию дальше
    traj_ALSN->setSignalInfoBwd(code_to_next, distance + trajectory->getLength(), liter);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryALSN::load_config(CfgReader &cfg)
{
    cfg.getDouble("ALSN", "Frequency", frequency);
}

GET_TRAJECTORY_DEVICE(TrajectoryALSN)
