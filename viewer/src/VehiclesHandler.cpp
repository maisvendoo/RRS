#include "VehiclesHandler.h"

#include "Logger.h"
#include "settings.h"
#include "simulator-info-struct.h"
#include "simulator-update-struct.h"
#include "sound-manager.h"
#include "VehicleExterior.h"

#include <vsg/app/Viewer.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/MatrixTransform.h>

#include <QObject>
#include <QString>

#include <array>
#include <cstddef>
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
int VehiclesHandler::getCurrentTrainIndex() const noexcept
{
    if (cur_vehicle < 0 || static_cast<std::size_t>(cur_vehicle) >= vehicles.size())
        return -1;
    return vehicles[cur_vehicle].train_id;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::isUpdated() const noexcept
{
    return pos_count >= 3;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
simulator_time_t *VehiclesHandler::getDateTime()
{
    return (pos_count.load(std::memory_order_relaxed) >= 1)
        ? &pos_buf[pos_read % POS_BUF_SIZE].sim_time : nullptr;
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
    ref_time.store(t, std::memory_order_relaxed);
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

    const double client_time = ref_time + time_difference.load(std::memory_order_relaxed);

    // Advance read head so pos_read is the first frame >= client_time
    advanceInterpolation(client_time);

    // Swap state double buffer
    const bool update_state = is_new_state;
    if (update_state)
    {
        std::swap(state_front, state_back);
        is_new_state = false;
    }

    // Interframe interpolation — clamp to [0,1] to prevent extrapolation overshoot
    const auto& frame_cur  = pos_buf[pos_read      % POS_BUF_SIZE];
    const auto& frame_prev = pos_buf[pos_read_prev  % POS_BUF_SIZE];
    const double upd_dt = frame_cur.sim_time.simulation_seconds - frame_prev.sim_time.simulation_seconds;
    const double r_raw = (upd_dt > 0.0) ? (client_time - frame_prev.sim_time.simulation_seconds) / upd_dt : 0.0;
    const double r = std::clamp(r_raw, 0.0, 1.0);
    const double k = (1.0 - r);

    for (std::size_t i = 0; i < vehicles.size(); ++i)
    {
        vehicles[i].position = vsg::dvec3(
            k * frame_prev.vehicles[i].position_x + r * frame_cur.vehicles[i].position_x,
            k * frame_prev.vehicles[i].position_y + r * frame_cur.vehicles[i].position_y,
            k * frame_prev.vehicles[i].position_z + r * frame_cur.vehicles[i].position_z
        );

        vehicles[i].orth = vsg::normalize(vsg::dvec3(
            k * frame_prev.vehicles[i].orth_x + r * frame_cur.vehicles[i].orth_x,
            k * frame_prev.vehicles[i].orth_y + r * frame_cur.vehicles[i].orth_y,
            k * frame_prev.vehicles[i].orth_z + r * frame_cur.vehicles[i].orth_z
        ));

        vehicles[i].up = vsg::normalize(vsg::dvec3(
            k * frame_prev.vehicles[i].up_x + r * frame_cur.vehicles[i].up_x,
            k * frame_prev.vehicles[i].up_y + r * frame_cur.vehicles[i].up_y,
            k * frame_prev.vehicles[i].up_z + r * frame_cur.vehicles[i].up_z
        ));

        vehicles[i].right = vsg::cross(vehicles[i].orth, vehicles[i].up);

        const vsg::dmat4 rotate_matrix{vehicles[i].right.x,vehicles[i].right.y,vehicles[i].right.z,0.0,
                                       vehicles[i].orth.x, vehicles[i].orth.y, vehicles[i].orth.z, 0.0,
                                       vehicles[i].up.x,   vehicles[i].up.y,   vehicles[i].up.z,   0.0,
                                       0.0,                0.0,                0.0,                1.0};

        // Apply vehicle body matrix transform
        vehicles[i].transform->matrix = vsg::translate(vehicles[i].position) * rotate_matrix;
        vehicles[i].cullnode->bound.center = vehicles[i].position;

        if (upd_dt > 0.0)
        {
            vehicles[i].velocity = vsg::dvec3(
                (frame_cur.vehicles[i].position_x - frame_prev.vehicles[i].position_x) / upd_dt,
                (frame_cur.vehicles[i].position_y - frame_prev.vehicles[i].position_y) / upd_dt,
                (frame_cur.vehicles[i].position_z - frame_prev.vehicles[i].position_z) / upd_dt
            );
        }

        // Model animations update and step
        if (update_state)
        {
            vehicles[i].train_id = state_front.vehicles[i].train_id;
            vehicles[i].orientation = state_front.vehicles[i].orientation;
            vehicles[i].prev_vehicle = state_front.vehicles[i].prev_vehicle;
            vehicles[i].next_vehicle = state_front.vehicles[i].next_vehicle;

            vehicles[i].step(static_cast<float>(t), static_cast<float>(dt), &(state_front.vehicles[i].analogSignal));
        }
        else
        {
            vehicles[i].step(static_cast<float>(t), static_cast<float>(dt));
        }

        // Sounds update
        for (auto sound_id : vehicles[i].sounds_id)
        {
            const vsg::vec3 pos = vsg::vec3(vehicles[i].position) +
                                  vsg::vec3(vehicles[i].right) * sound_manager->getLocalPositionX(sound_id) +
                                  vsg::vec3(vehicles[i].orth) * sound_manager->getLocalPositionY(sound_id) +
                                  vsg::vec3(vehicles[i].up) * sound_manager->getLocalPositionZ(sound_id);
            sound_manager->setPosition(sound_id, pos.x, pos.y, pos.z);

            if (update_state)
            {
                sound_manager->setVelocity(sound_id, vehicles[i].velocity.x, vehicles[i].velocity.y, vehicles[i].velocity.z);

                const std::size_t signal_id = sound_manager->getSignalID(sound_id);
                if (signal_id < state_front.vehicles[i].analogSignal.size())
                    sound_manager->setSoundSignal(sound_id, state_front.vehicles[i].analogSignal[signal_id]);
                else
                    sound_manager->setSoundSignal(sound_id, 0.0f);
            }
        }
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
        const int new_train_id = update_trains.trains.size() - 1;
        cur_vehicle = update_trains.trains[new_train_id].first_vehicle_id;
    }
    else
    {
        const int new_train_id = vehicles[cur_vehicle].train_id - 1;
        cur_vehicle = update_trains.trains[new_train_id].first_vehicle_id;
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
    if (static_cast<std::size_t>(vehicles[cur_vehicle].train_id) >= (update_trains.trains.size() - 1))
    {
        cur_vehicle = update_trains.trains[0].first_vehicle_id;
    }
    else
    {
        const int new_train_id = vehicles[cur_vehicle].train_id + 1;
        cur_vehicle = update_trains.trains[new_train_id].first_vehicle_id;
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
        cur_vehicle = update_trains.trains[cur_train_id].last_vehicle_id;
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
        cur_vehicle = update_trains.trains[cur_train_id].first_vehicle_id;
    }

    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::selectControlVehicle() noexcept
{
    auto vehicle = getCurrentVehicle();

    if (vehicle)
    {
        const int prev_contr_vehicle = controlled_vehicle;
        const int prev_contr_cabine = vehicle->controlled_cabine_idx;

        // Берём контроль над данным вагоном
        controlled_vehicle = cur_vehicle;
        // Берём контроль над данной кабиной
        vehicle->controlled_cabine_idx = vehicle->current_cabine_idx;

        return (controlled_vehicle != prev_contr_vehicle) ||
               (vehicle->controlled_cabine_idx != prev_contr_cabine);
    }
    return false;
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
        const double veh_len = vehicles_info.vehicles[i].vehicle_length;

        vehicles.emplace_back(VehicleExterior());
        VehicleExterior& vehicle_exterior = vehicles.back();
        vehicle_exterior.driver_pos[vehicle_exterior.current_cabine_idx] = settings.cabine_default_pos;
        vehicle_exterior.saved_cabine_cam_fov = settings.fovy;

        if (vehicle_exterior.loadVehicle(cfg_dir, cfg_file, sound_manager, options))
        {
            LOG_INFO("Added vehicle %u / %u with model from %s / %s.xml",
                     i + 1, vehicle_count, cfg_dir.c_str(), cfg_file.c_str());
        }
        else
        {
            LOG_WARN("Added vehicle %u / %u. Fail to load model from %s / %s.xml",
                     i + 1, vehicle_count, cfg_dir.c_str(), cfg_file.c_str());
        }

        vehicle_exterior.cullnode->bound = vsg::dsphere(0.0, 0.0, 0.0, veh_len);
        vehicle_exterior.cullnode->child = vehicle_exterior.transform;
        vehicles_node->addChild(vehicle_exterior.cullnode);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::slotGetTrainsData(QByteArray &data)
{
    update_trains.deserialize(data);
/*
    QString msg = "";
    msg += "Trains(";
    msg += QString::number(update_trains.trains.size());
    msg += "):";
    for (size_t i = 0; i < update_trains.trains.size(); ++i)
    {
        msg += "\n";
        msg += update_trains.trains[i].train_name;
        msg += ":";
        msg += QString::number(update_trains.trains[i].first_vehicle_id);
        msg += ",";
        msg += QString::number(update_trains.trains[i].last_vehicle_id);
    }
    LOG_INFO("%s", msg.toStdString().c_str());
*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::slotGetVehiclesPosData(QByteArray& data)
{
    const size_t slot = pos_write.load(std::memory_order_relaxed) % POS_BUF_SIZE;
    pos_buf[slot].deserialize(data);

    if (pos_buf[slot].vehicles.size() != vehicles.size())
    {
        LOG_WARN("Fail to update: get %zu positions but there are %zu vehicles",
                 pos_buf[slot].vehicles.size(), vehicles.size());
        return;
    }

    // Exponential smoothing of time offset (converges quickly during startup)
    const size_t count = pos_count.load(std::memory_order_relaxed);
    const double alpha = (count < 3) ? 0.5 : 0.05;
    const double td = time_difference.load(std::memory_order_relaxed);
    const double rt = ref_time.load(std::memory_order_relaxed);
    time_difference.store(td * (1.0 - alpha) +
        (pos_buf[slot].sim_time.simulation_seconds - rt - settings_delay) * alpha,
        std::memory_order_relaxed);

    // Publish: data is fully written, now make it visible to the reader
    const size_t new_write = pos_write.load(std::memory_order_relaxed) + 1;
    pos_write.store(new_write, std::memory_order_release);
    if (count < POS_BUF_SIZE)
        pos_count.store(count + 1, std::memory_order_release);

    // Initialize read heads once we have enough frames
    if (count + 1 == 3)
    {
        pos_read_prev = new_write - 3;
        pos_read      = new_write - 2;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::slotGetVehiclesStateData(QByteArray& data)
{
    if (is_new_state)
        return;

    state_back.deserialize(data);
    if (state_back.vehicles.size() == vehicles.size())
    {
        is_new_state = true;
    }
    else
    {
        LOG_WARN("Fail to update: get %zu states but there are %zu vehicles",
                 state_back.vehicles.size(), vehicles.size());
        is_new_state = false;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::slotGetVehicleControlled(QByteArray& data)
{
    if (!isUpdated())
        return;

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
//
//------------------------------------------------------------------------------
void VehiclesHandler::advanceInterpolation(double client_time)
{
    // Advance read head until pos_read is the first frame with time >= client_time
    // but don't go past the latest written frame.
    // pos_write is atomic — snapshot it once to avoid torn reads in the loop.
    const size_t write_snapshot = pos_write.load(std::memory_order_acquire);
    if (write_snapshot == 0)
        return;
    const size_t latest = write_snapshot - 1;
    while (pos_read < latest &&
           client_time >= pos_buf[pos_read % POS_BUF_SIZE].sim_time.simulation_seconds)
    {
        pos_read_prev = pos_read;
        ++pos_read;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::updateDebugString()
{
    const size_t w = pos_write.load(std::memory_order_acquire);
    if (w == 0)
        return;

    auto& latest = pos_buf[(w - 1) % POS_BUF_SIZE];

    // Дата-время сервера
    debug_message = latest.sim_time.getString() + "\n";

    const int current = vehicle_controlled.current_vehicle;
    if (current >= 0
        && static_cast<std::size_t>(current) < state_front.vehicles.size()
        && static_cast<std::size_t>(current) < latest.vehicles.size())
    {
        const int current_train = state_front.vehicles[current].train_id;
        const auto& new_pos_data = latest.vehicles[current];
        debug_message += QString("Данная ПЕ: %1 | Поезд %2 | pos{%3,%4,%5} | dir{%6,%7,%8}\n")
            .arg(current, 3)
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

    const int control = vehicle_controlled.controlled_vehicle;
    if (control >= 0
        && control < state_front.vehicles.size()
        && control < latest.vehicles.size())
    {
        const int control_train = state_front.vehicles[control].train_id;
        const auto& new_pos_data = latest.vehicles[control];
        debug_message += QString("\nУправляемая ПЕ: %1 | Поезд %2 | pos{%3,%4,%5} | dir{%6,%7,%8}\n")
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
        debug_message += QString("\nУправляемая ПЕ: не выбрана\nНажмите Enter, чтобы управлять данной ПЕ");
    }
}
