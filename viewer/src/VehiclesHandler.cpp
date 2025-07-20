#include "VehiclesHandler.h"

#include "Logger.h"
#include "ProcAnimation.h"
#include "settings.h"
#include "simulator-info-struct.h"
#include "simulator-update-struct.h"
#include "sound-manager.h"
#include "VehicleExterior.h"

#include <algorithm>
#include <vsg/app/Viewer.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Group.h>

#include <QObject>
#include <QString>

#include <array>
#include <cmath>
#include <cstddef>
#include <functional>
#include <string>
#include <utility>
#include <vector>

class QByteArray;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehiclesHandler::VehiclesHandler(const settings_t& settings, SoundManager* sound_manager, QObject* parent)
    : QObject(parent)
    , sound_manager(sound_manager)
{
    settings_delay = (settings.vehicle_controled_update_interval + settings.client_delay) * 0.001;
    current_get_vehicles_pos_data_function = [&](QByteArray& data) { getVehiclesPosData1(data); };
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Group> VehiclesHandler::getExterior() const noexcept
{
    return vehicles_node;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleExterior* VehiclesHandler::getCurrentVehicle()
{
#ifndef NDEBUG
    if (isUpdated())
    {
        if (cur_vehicle >= 0 && static_cast<std::size_t>(cur_vehicle) < vehicles.size())
        {
            return &(vehicles[cur_vehicle]);
        }
        else
        {
            LOG_WARN("cur_vehicle(%d) is not in range of vehicles.size(%d)", cur_vehicle, vehicles.size());
            return nullptr;
        }
    }
    else
    {
        return nullptr;
    }
#else
    return isUpdated() ? &(vehicles[cur_vehicle]) : nullptr;
#endif
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int VehiclesHandler::getCurrentVehicleIndex() const noexcept
{
    return cur_vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int VehiclesHandler::getControlledVehicleIndex() const noexcept
{
    return controlled_vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::isUpdated() const noexcept
{
    return is_pos_updated && is_state_updated;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString VehiclesHandler::getDebugMessage() const noexcept
{
    return debug_message;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::step(double t, double dt)
{
    ref_time = t;
    if (isUpdated())
    {
        if (!is_updated)
        {
            is_updated = true;
            emit updated();
        }
    }
    else
    {
        is_updated = false;
        return;
    }

    const double client_time = ref_time + time_difference;
    const bool is_update = (client_time >= update_pos_data[cur_data].sim_time.simulation_seconds);

    // Swap indexes of positions info array
    while (is_update)
    {
        // Check for use the latest data already
        if (cur_data == delay_data)
        {
            delay_data = new_data;

            if (cur_data == delay_data)
            {
                // No new data, the latest data is already used
                break;
            }
        }

        // Update index
        unused_data = (cur_data == 0) ? (DATA_ARRAY_SIZE - 1) : (cur_data - 1);
        old_data = cur_data;
        cur_data = delay_data;
        delay_data = new_data;

        if (client_time < update_pos_data[cur_data].sim_time.simulation_seconds)
        {
            break;
        }
    }

    // Save state update flag for this frame update
    const bool update_state = is_new_state;
    if (update_state)
    {
        // Swap indexes of states info array
        std::swap(new_state, unused_state);

        // Reset state update flag
        is_new_state = false;
    }

    // Interframe coordinate
    const double upd_dt = update_pos_data[cur_data].sim_time.simulation_seconds - update_pos_data[old_data].sim_time.simulation_seconds;
    const double r = (client_time - update_pos_data[old_data].sim_time.simulation_seconds) / upd_dt;
    const double k = (1.0 - r);

    for (std::size_t i = 0; i < vehicles.size(); ++i)
    {
        vehicles[i].position = vsg::dvec3(
            k * update_pos_data[old_data].vehicles[i].position_x + r * update_pos_data[cur_data].vehicles[i].position_x,
            k * update_pos_data[old_data].vehicles[i].position_y + r * update_pos_data[cur_data].vehicles[i].position_y,
            k * update_pos_data[old_data].vehicles[i].position_z + r * update_pos_data[cur_data].vehicles[i].position_z
        );

        vehicles[i].orth = vsg::normalize(vsg::dvec3(
            k * update_pos_data[old_data].vehicles[i].orth_x + r * update_pos_data[cur_data].vehicles[i].orth_x,
            k * update_pos_data[old_data].vehicles[i].orth_y + r * update_pos_data[cur_data].vehicles[i].orth_y,
            k * update_pos_data[old_data].vehicles[i].orth_z + r * update_pos_data[cur_data].vehicles[i].orth_z
        ));

        vehicles[i].up = vsg::normalize(vsg::dvec3(
            k * update_pos_data[old_data].vehicles[i].up_x + r * update_pos_data[cur_data].vehicles[i].up_x,
            k * update_pos_data[old_data].vehicles[i].up_y + r * update_pos_data[cur_data].vehicles[i].up_y,
            k * update_pos_data[old_data].vehicles[i].up_z + r * update_pos_data[cur_data].vehicles[i].up_z
        ));

        vehicles[i].right = vsg::cross(vehicles[i].orth, vehicles[i].up);

        vehicles[i].attitude = vsg::dvec3(
            std::asin(vehicles[i].orth.z),
            0.0,
            (vehicles[i].orth.x > 0.0) ? std::acos(vehicles[i].orth.y) : -std::acos(vehicles[i].orth.y)
        );

        // Apply vehicle body matrix transform
        vehicles[i].transform->matrix = vsg::translate(vehicles[i].position) *
            vsg::rotate(-vehicles[i].attitude.z, vsg::dvec3(0.0, 0.0, 1.0)) *
            vsg::rotate(vehicles[i].attitude.x, vsg::dvec3(1.0, 0.0, 0.0));

        if (is_update)
        {
            vehicles[i].velocity = vsg::dvec3(
                (update_pos_data[cur_data].vehicles[i].position_x - update_pos_data[old_data].vehicles[i].position_x) / upd_dt,
                (update_pos_data[cur_data].vehicles[i].position_y - update_pos_data[old_data].vehicles[i].position_y) / upd_dt,
                (update_pos_data[cur_data].vehicles[i].position_z - update_pos_data[old_data].vehicles[i].position_z) / upd_dt
            );
        }

        if (update_state)
        {
            vehicles[i].train_id = update_data[new_state].vehicles[i].train_id;
            vehicles[i].orientation = update_data[new_state].vehicles[i].orientation;
            vehicles[i].prev_vehicle = update_data[new_state].vehicles[i].prev_vehicle;
            vehicles[i].next_vehicle = update_data[new_state].vehicles[i].next_vehicle;

            // Model animations update
            for (auto& [signal_id, animation] : vehicles[i].animations->animations)
            {
                animation->setSignals(&(update_data[new_state].vehicles[i].analogSignal));
            }

            // Sounds update
            for (auto sound_id : vehicles[i].sounds_id)
            {
                const vsg::vec3 pos = vsg::vec3(vehicles[i].position) +
                                      vsg::vec3(vehicles[i].right) * sound_manager->getLocalPositionX(sound_id) +
                                      vsg::vec3(vehicles[i].orth) * sound_manager->getLocalPositionY(sound_id) +
                                      vsg::vec3(vehicles[i].up) * sound_manager->getLocalPositionZ(sound_id);
                sound_manager->setPosition(sound_id, pos.x, pos.y, pos.z);
                sound_manager->setVelocity(sound_id, vehicles[i].velocity.x, vehicles[i].velocity.y, vehicles[i].velocity.z);

                const std::size_t signal_id = sound_manager->getSignalID(sound_id);
                if (signal_id < update_data[new_state].vehicles[i].analogSignal.size())
                {
                    sound_manager->setSoundSignal(sound_id, update_data[new_state].vehicles[i].analogSignal[signal_id]);
                }
                else
                {
                    sound_manager->setSoundSignal(sound_id, 0.0f);
                }
            }
        }
        else
        {
            for (auto sound_id : vehicles[i].sounds_id)
            {
                const vsg::vec3 pos = vsg::vec3(vehicles[i].position) +
                                      vsg::vec3(vehicles[i].right) * sound_manager->getLocalPositionX(sound_id) +
                                      vsg::vec3(vehicles[i].orth) * sound_manager->getLocalPositionY(sound_id) +
                                      vsg::vec3(vehicles[i].up) * sound_manager->getLocalPositionZ(sound_id);
                sound_manager->setPosition(sound_id, pos.x, pos.y, pos.z);
            }
        }

        // Model animations step
        vehicles[i].step(static_cast<float>(t), static_cast<float>(dt), camera_pos);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::selectNextTrain() noexcept
{
    const int prev_cur_vehicle = cur_vehicle;

    // Переключаем на первый вагон предыдущего поезда
    if (vehicles[cur_vehicle].train_id <= 0)
    {
        const int new_train_id = update_data[new_state].trains.size() - 1;
        cur_vehicle = update_data[new_state].trains[new_train_id].first_vehicle_id;
    }
    else
    {
        const int new_train_id = vehicles[cur_vehicle].train_id - 1;
        cur_vehicle = update_data[new_state].trains[new_train_id].first_vehicle_id;
    }

    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::selectPrevTrain() noexcept
{
    const int prev_cur_vehicle = cur_vehicle;

    // Переключаем на первый вагон следующего поезда
    if (static_cast<std::size_t>(vehicles[cur_vehicle].train_id) >= (update_data[new_state].trains.size() - 1))
    {
        cur_vehicle = update_data[new_state].trains[0].first_vehicle_id;
    }
    else
    {
        const int new_train_id = vehicles[cur_vehicle].train_id + 1;
        cur_vehicle = update_data[new_state].trains[new_train_id].first_vehicle_id;
    }

    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::selectNextVehicle() noexcept
{
    const int prev_cur_vehicle = cur_vehicle;

    // Переключение по вагонам поезда вперёд
    if (vehicles[cur_vehicle].prev_vehicle >= 0)
    {
        cur_vehicle = vehicles[cur_vehicle].prev_vehicle;
    }
    else
    {
        // С первого вагона переключаемся на последний
        const int cur_train_id = vehicles[cur_vehicle].train_id;
        cur_vehicle = update_data[new_state].trains[cur_train_id].last_vehicle_id;
    }

    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::selectPrevVehicle() noexcept
{
    const int prev_cur_vehicle = cur_vehicle;

    // Переключение по вагонам поезда назад
    if (vehicles[cur_vehicle].next_vehicle >= 0)
    {
        cur_vehicle = vehicles[cur_vehicle].next_vehicle;
    }
    else
    {
        // С последнего вагона переключаемся на первый
        const int cur_train_id = vehicles[cur_vehicle].train_id;
        cur_vehicle = update_data[new_state].trains[cur_train_id].first_vehicle_id;
    }

    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::selectControlVehicle() noexcept
{
    // Берём контроль над данным вагоном
    controlled_vehicle = cur_vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::returnToControlledVehicle() noexcept
{
    const int prev_cur_vehicle = cur_vehicle;

    // Возврат к управляемому вагону
    cur_vehicle = controlled_vehicle;

    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::load(
    QByteArray& data,
    const settings_t& settings,
    vsg::ref_ptr<vsg::Viewer> viewer,
    vsg::ref_ptr<vsg::Options> options
)
{
    vehicles_info.deserialize(data);
    if (vehicles_info.vehicles.empty())
    {
        LOG_WARN("Server has not any vehicles");
        return false;
    }

    const std::size_t vehicle_count = vehicles_info.vehicles.size();
    LOG_INFO("Got info about %u vehicles from server", vehicle_count);

    vehicles.reserve(vehicle_count);
    vehicles_node->children.reserve(vehicle_count);

    for (std::size_t i = 0; i < vehicle_count; ++i)
    {
        const std::string cfg_dir = vehicles_info.vehicles[i].vehicle_config_dir.toStdString();
        const std::string cfg_file = vehicles_info.vehicles[i].vehicle_config_file.toStdString();

        VehicleExterior vehicle_exterior;
        vehicle_exterior.driver_pos[vehicle_exterior.cabine_idx_ref] = settings.cabine_default_pos;
        vehicle_exterior.saved_cabine_cam_fov = settings.fovy;

        if (vehicle_exterior.loadVehicle(cfg_dir, cfg_file, sound_manager, viewer, options))
        {
            LOG_INFO("Added vehicle %u / %u with model from %s / %s.xml",
                     i + 1, vehicle_count, cfg_dir.c_str(), cfg_file.c_str());
        }
        else
        {
            LOG_WARN("Added vehicle %u / %u. Fail to load model from %s / %s.xml",
                     i + 1, vehicle_count, cfg_dir.c_str(), cfg_file.c_str());
        }

        auto vehicle_exterior_transform = vehicle_exterior.transform;
        vehicles.emplace_back(std::move(vehicle_exterior));
        vehicles_node->addChild(vehicle_exterior_transform);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::slotGetVehiclesPosData(QByteArray& data)
{
    current_get_vehicles_pos_data_function(data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::slotGetVehiclesStateData(QByteArray& data)
{
    if (is_new_state)
    {
        return;
    }

    update_data[unused_state].deserialize(data);
    if (update_data[unused_state].vehicles.size() == vehicles.size())
    {
/*
        QString msg = "";
        msg += "\nTrains(";
        msg += QString::number(update_data[unused_state].trains.size());
        msg += "):";
        for (size_t i = 0; i < update_data[unused_state].trains.size(); ++i)
        {
            msg += QString::number(update_data[unused_state].trains[i].first_vehicle_id);
            msg += ",";
            msg += QString::number(update_data[unused_state].trains[i].last_vehicle_id);
            msg += "|";
        }
        msg += "\nVehicles(";
        msg += QString::number(update_data[unused_state].vehicles.size());
        msg += "):";
        for (size_t i = 0; i < update_data[unused_state].vehicles.size(); ++i)
        {
            msg += "\n(";
            msg += QString::number(update_data[unused_state].vehicles[i].train_id);
            msg += ")";
            msg += QString::number(i);
            msg += "(";
            msg += QString::number(update_data[unused_state].vehicles[i].analogSignal.size());
            msg += "):";
            for (auto s : update_data[unused_state].vehicles[i].analogSignal)
            {
                msg += QString::number(s);
                msg += "|";
            }
        }
        LOG_INFO("%s", msg.toStdString().c_str());
*/
        is_state_updated = true;
        is_new_state = true;
    }
    else
    {
        LOG_WARN("Fail to update: get %u states but there are %u vehicles",
                 update_data[unused_state].vehicles.size(),
                 vehicles.size());

        is_state_updated = false;
        is_new_state = false;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::slotGetVehicleControlled(QByteArray& data)
{
    if (!isUpdated())
    {
        return;
    }

    vehicle_controlled.deserialize(data);
    if ((vehicle_controlled.controlled_vehicle >= 0) &&
        (static_cast<std::size_t>(vehicle_controlled.controlled_vehicle) < vehicles.size()) &&
        (vehicle_controlled.current_vehicle >= 0) &&
        (static_cast<std::size_t>(vehicle_controlled.current_vehicle) < vehicles.size()))
    {
        updateDebugString();
    }
}

//------------------------------------------------------------------------------
// Первое получение данных
//------------------------------------------------------------------------------
void VehiclesHandler::getVehiclesPosData1(QByteArray& data)
{
    new_data = DATA_ARRAY_SIZE - 3;
    update_pos_data[new_data].deserialize(data);
    if (update_pos_data[new_data].vehicles.size() == vehicles.size())
    {
        time_difference = update_pos_data[new_data].sim_time.simulation_seconds - ref_time - settings_delay;
        current_get_vehicles_pos_data_function = [&](QByteArray& data) { getVehiclesPosData2(data); };
    }
    else
    {
        LOG_WARN("Fail to update: get %u positions but there are %u vehicles",
                    update_pos_data[new_data].vehicles.size(),
                    vehicles.size());
        new_data = -1;
    }
}

//------------------------------------------------------------------------------
// Второе получение данных
//------------------------------------------------------------------------------
void VehiclesHandler::getVehiclesPosData2(QByteArray& data)
{
    delay_data = new_data;
    new_data = DATA_ARRAY_SIZE - 2;
    update_pos_data[new_data].deserialize(data);
    if (update_pos_data[new_data].vehicles.size() == vehicles.size())
    {
        double r = 0.5;
        time_difference = time_difference * (1.0 - r) +
            (update_pos_data[new_data].sim_time.simulation_seconds - ref_time - settings_delay) * r;
        current_get_vehicles_pos_data_function = [&](QByteArray& data) { getVehiclesPosData3(data); };
    }
    else
    {
        LOG_WARN("Fail to update: get %u positions but there are %u vehicles",
                    update_pos_data[new_data].vehicles.size(),
                    vehicles.size());
        new_data = -1;
        delay_data = -1;
        current_get_vehicles_pos_data_function = [&](QByteArray& data) { getVehiclesPosData1(data); };
    }
}

//------------------------------------------------------------------------------
// Третье получение данных
//------------------------------------------------------------------------------
void VehiclesHandler::getVehiclesPosData3(QByteArray& data)
{
    new_data = DATA_ARRAY_SIZE - 1;
    update_pos_data[new_data].deserialize(data);

    is_pos_updated = (update_pos_data[new_data].vehicles.size() == vehicles.size());
    if (is_pos_updated)
    {
        unused_data = DATA_ARRAY_SIZE - 4;
        old_data = DATA_ARRAY_SIZE - 3;
        cur_data = DATA_ARRAY_SIZE - 2;
        delay_data = DATA_ARRAY_SIZE - 1;

        double r = 0.25;
        time_difference = time_difference * (1.0 - r) +
            (update_pos_data[new_data].sim_time.simulation_seconds - ref_time - settings_delay) * r;
        current_get_vehicles_pos_data_function = [&](QByteArray& data) { getVehiclesPosData4(data); };
    }
    else
    {
        LOG_WARN("Fail to update: get %u positions but there are %u vehicles",
                 update_pos_data[new_data].vehicles.size(),
                 vehicles.size());
        new_data = -1;
        delay_data = -1;
        current_get_vehicles_pos_data_function = [&](QByteArray& data) { getVehiclesPosData1(data); };
    }
}

//------------------------------------------------------------------------------
// Обновление данных по очереди
//------------------------------------------------------------------------------
void VehiclesHandler::getVehiclesPosData4(QByteArray& data)
{
    ++new_data;
    if (new_data >= DATA_ARRAY_SIZE)
    {
        new_data = 0;
    }

    // Не даём обновлениям догнать цикл сзади
    if (new_data == old_data)
    {
        new_data = unused_data;
    }

    update_pos_data[new_data].deserialize(data);

    is_pos_updated = (update_pos_data[new_data].vehicles.size() == vehicles.size());

    if (is_pos_updated)
    {
        double r = 0.05;
        time_difference = time_difference * (1.0 - r) +
            (update_pos_data[new_data].sim_time.simulation_seconds - ref_time - settings_delay) * r;
    }
    else
    {
        LOG_WARN("Fail to update: get %u positions but there are %u vehicles",
                 update_pos_data[new_data].vehicles.size(),
                 vehicles.size());
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::updateDebugString()
{
    debug_message = QString("Время сервера: %1-%2-%3 %4:%5:%6 (%7 с)\n")
                        .arg(update_pos_data[new_data].sim_time.date.year(), 4, 10, QChar('0'))
                        .arg(update_pos_data[new_data].sim_time.date.month(), 2, 10, QChar('0'))
                        .arg(update_pos_data[new_data].sim_time.date.day(), 2, 10, QChar('0'))
                        .arg(update_pos_data[new_data].sim_time.time.hour(), 2, 10, QChar('0'))
                        .arg(update_pos_data[new_data].sim_time.time.minute(), 2, 10, QChar('0'))
                        .arg(update_pos_data[new_data].sim_time.time.sec(), 2, 10, QChar('0'))
                        .arg(update_pos_data[new_data].sim_time.simulation_seconds, 3, 'f', 1);

    const int current_vehicle = vehicle_controlled.current_vehicle;
    if (current_vehicle >= 0
        && static_cast<std::size_t>(current_vehicle) < update_data[new_state].vehicles.size()
        && static_cast<std::size_t>(current_vehicle) < update_pos_data[new_data].vehicles.size())
    {
        const int current_train = update_data[new_state].vehicles[current_vehicle].train_id;
        const auto& new_pos_data = update_pos_data[new_data].vehicles[current_vehicle];
        debug_message += QString("Данная ПЕ: %1 | Поезд %2 | pos{%3,%4,%5} | dir{%6,%7,%8}\n")
            .arg(current_vehicle, 3)
            .arg(current_train, 3)
            .arg(new_pos_data.position_x, 8, 'f', 1)
            .arg(new_pos_data.position_y, 8, 'f', 1)
            .arg(new_pos_data.position_z, 8, 'f', 1)
            .arg(new_pos_data.orth_x, 6, 'f', 3)
            .arg(new_pos_data.orth_y, 6, 'f', 3)
            .arg(new_pos_data.orth_z, 6, 'f', 3);

        debug_message += vehicle_controlled.currentDebugMsg + QString("\n");
    }
    else
    {
        debug_message += QString("\n\n");
    }

    const std::size_t control = vehicle_controlled.controlled_vehicle;
    if (control >= 0
        && control < update_data[new_state].vehicles.size()
        && control < update_pos_data[new_data].vehicles.size())
    {
        const int control_train = update_data[new_state].vehicles[control].train_id;
        const auto& new_pos_data = update_pos_data[new_data].vehicles[control];
        debug_message += QString("Управляемая ПЕ: %1 | Поезд %2 | pos{%3,%4,%5} | dir{%6,%7,%8}\n")
            .arg(control, 3)
            .arg(control_train, 3)
            .arg(new_pos_data.position_x, 8, 'f', 1)
            .arg(new_pos_data.position_y, 8, 'f', 1)
            .arg(new_pos_data.position_z, 8, 'f', 1)
            .arg(new_pos_data.orth_x, 6, 'f', 3)
            .arg(new_pos_data.orth_y, 6, 'f', 3)
            .arg(new_pos_data.orth_z, 6, 'f', 3);

        debug_message += vehicle_controlled.controlledDebugMsg;
    }
    else
    {
        debug_message += QString("Управляемая ПЕ: не выбрана\nНажмите Enter, чтобы управлять данной ПЕ");
    }
}
