#include "VehiclesHandler.h"

#include <vsg/maths/transform.h>
#include <vsg/io/stream.h>
#include <vsg/io/Options.h>

#include "ProcAnimation.h"
#include "VehicleExterior.h"
#include "settings.h"
#include "sound-manager.h"
#include "Logger.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehiclesHandler::VehiclesHandler(const settings_t& settings, SoundManager *sm, QObject *parent)
    : QObject(parent)
    , sound_manager(sm)
{
    settings_delay = (settings.vehicle_controled_update_interval + settings.client_delay) * 0.001;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Group> VehiclesHandler::getExterior()
{
    return vehicles_node;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleExterior *VehiclesHandler::getCurrentVehicle()
{
    if (isUpdated())
        return &(vehicles[cur_vehicle]);

    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int VehiclesHandler::getCurrentVehicleIndex()
{
    return cur_vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int VehiclesHandler::getControlledVehicleIndex()
{
    return controlled_vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::isUpdated()
{
    return is_pos_updated && is_state_updated;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::step(double t, double dt)
{
    ref_time = t;
    if (!isUpdated())
        return;

    double client_time = ref_time + time_difference;
    bool is_update = (client_time >= update_pos_data[cur_data].time);

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

        if (client_time < update_pos_data[cur_data].time)
        {
            break;
        }
    }

    // Swap indexes of states info array
    if (is_new_state)
        std::swap(new_state, unused_state);

    // Interframe coordinate
    double upd_dt = update_pos_data[cur_data].time - update_pos_data[old_data].time;
    double r = (client_time - update_pos_data[old_data].time) / upd_dt;
    double k = (1.0 - r);

    for (size_t i = 0; i < vehicles.size(); ++i)
    {
        vehicles[i].position = vsg::dvec3(
            k * update_pos_data[old_data].vehicles[i].position_x + r * update_pos_data[cur_data].vehicles[i].position_x,
            k * update_pos_data[old_data].vehicles[i].position_y + r * update_pos_data[cur_data].vehicles[i].position_y,
            k * update_pos_data[old_data].vehicles[i].position_z + r * update_pos_data[cur_data].vehicles[i].position_z);

        vehicles[i].orth = normalize(vsg::dvec3(
            k * update_pos_data[old_data].vehicles[i].orth_x + r * update_pos_data[cur_data].vehicles[i].orth_x,
            k * update_pos_data[old_data].vehicles[i].orth_y + r * update_pos_data[cur_data].vehicles[i].orth_y,
            k * update_pos_data[old_data].vehicles[i].orth_z + r * update_pos_data[cur_data].vehicles[i].orth_z));

        vehicles[i].up = normalize(vsg::dvec3(
            k * update_pos_data[old_data].vehicles[i].up_x + r * update_pos_data[cur_data].vehicles[i].up_x,
            k * update_pos_data[old_data].vehicles[i].up_y + r * update_pos_data[cur_data].vehicles[i].up_y,
            k * update_pos_data[old_data].vehicles[i].up_z + r * update_pos_data[cur_data].vehicles[i].up_z));

        vehicles[i].right = cross(vehicles[i].orth, vehicles[i].up);

        vehicles[i].attitude = vsg::dvec3(
            asin(vehicles[i].orth.z),
            0.0,
            (vehicles[i].orth.x > 0.0) ? acos(vehicles[i].orth.y) : - acos(vehicles[i].orth.y) );

        // Apply vehicle body matrix transform
        vehicles[i].transform->matrix = vsg::translate(vehicles[i].position) *
            vsg::rotate(-vehicles[i].attitude.z, vsg::dvec3(0.0, 0.0, 1.0)) *
            vsg::rotate(vehicles[i].attitude.x, vsg::dvec3(1.0, 0.0, 0.0));

        if (is_new_state)
        {
            vehicles[i].train_id = update_data[new_state].vehicles[i].train_id;
            vehicles[i].orientation = update_data[new_state].vehicles[i].orientation;
            vehicles[i].prev_vehicle = update_data[new_state].vehicles[i].prev_vehicle;
            vehicles[i].next_vehicle = update_data[new_state].vehicles[i].next_vehicle;

            // Model animations update
            for (auto animation : vehicles[i].animations)
            {
                size_t signal_id = animation.first;
                if (signal_id < update_data[new_state].vehicles[i].analogSignal.size())
                    animation.second->setPosition(update_data[new_state].vehicles[i].analogSignal[signal_id]);
                else
                    animation.second->setPosition(0.0f);
            }

            // Sounds update
            vsg::dvec3 offset = vsg::dvec3(
                (update_pos_data[cur_data].vehicles[i].position_x - update_pos_data[old_data].vehicles[i].position_x),
                (update_pos_data[cur_data].vehicles[i].position_y - update_pos_data[old_data].vehicles[i].position_y),
                (update_pos_data[cur_data].vehicles[i].position_z - update_pos_data[old_data].vehicles[i].position_z));
            vsg::dvec3 velocity = offset / upd_dt;

            for (auto sound_id : vehicles[i].sounds_id)
            {
                vsg::vec3 pos = vsg::vec3(vehicles[i].position) +
                                vsg::vec3(vehicles[i].right) * sound_manager->getLocalPositionX(sound_id) +
                                vsg::vec3(vehicles[i].orth) * sound_manager->getLocalPositionY(sound_id) +
                                vsg::vec3(vehicles[i].up) * sound_manager->getLocalPositionZ(sound_id);
                sound_manager->setPosition(sound_id, pos.x, pos.y, pos.z);
                sound_manager->setVelocity(sound_id, velocity.x, velocity.y, velocity.z);

                size_t signal_id = sound_manager->getSignalID(sound_id);
                if (signal_id < update_data[new_state].vehicles[i].analogSignal.size())
                    sound_manager->setSoundSignal(sound_id, update_data[new_state].vehicles[i].analogSignal[signal_id]);
                else
                    sound_manager->setSoundSignal(sound_id, 0.0f);
            }
        }
        else
        {
            for (auto sound_id : vehicles[i].sounds_id)
            {
                vsg::vec3 pos = vsg::vec3(vehicles[i].position) +
                                vsg::vec3(vehicles[i].right) * sound_manager->getLocalPositionX(sound_id) +
                                vsg::vec3(vehicles[i].orth) * sound_manager->getLocalPositionY(sound_id) +
                                vsg::vec3(vehicles[i].up) * sound_manager->getLocalPositionZ(sound_id);
                sound_manager->setPosition(sound_id, pos.x, pos.y, pos.z);
            }
        }

        // Model animations step
        vehicles[i].step(static_cast<float>(t), static_cast<float>(dt));
    }

    // Reset state update
    is_new_state = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::selectNextTrain()
{
    int prev_cur_vehicle = cur_vehicle;

    // Переключаем на первый вагон предыдущего поезда
    if (vehicles[cur_vehicle].train_id <= 0)
    {
        int new_train_id = update_data[new_state].trains.size() - 1;
        cur_vehicle = update_data[new_state].trains[new_train_id].first_vehicle_id;
    }
    else
    {
        int new_train_id = vehicles[cur_vehicle].train_id - 1;
        cur_vehicle = update_data[new_state].trains[new_train_id].first_vehicle_id;
    }
    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::selectPrevTrain()
{
    int prev_cur_vehicle = cur_vehicle;

    // Переключаем на первый вагон следующего поезда
    if (vehicles[cur_vehicle].train_id >= (update_data[new_state].trains.size() - 1))
    {
        cur_vehicle = update_data[new_state].trains[0].first_vehicle_id;
    }
    else
    {
        int new_train_id = vehicles[cur_vehicle].train_id + 1;
        cur_vehicle = update_data[new_state].trains[new_train_id].first_vehicle_id;
    }
    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::selectNextVehicle()
{
    int prev_cur_vehicle = cur_vehicle;

    // Переключение по вагонам поезда вперёд
    if (vehicles[cur_vehicle].prev_vehicle >= 0)
    {
        cur_vehicle = vehicles[cur_vehicle].prev_vehicle;
    }
    else
    {
        // С первого вагона переключаемся на последний
        int cur_train_id = vehicles[cur_vehicle].train_id;
        cur_vehicle = update_data[new_state].trains[cur_train_id].last_vehicle_id;
    }
    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::selectPrevVehicle()
{
    int prev_cur_vehicle = cur_vehicle;

    // Переключение по вагонам поезда назад
    if (vehicles[cur_vehicle].next_vehicle >= 0)
    {
        cur_vehicle = vehicles[cur_vehicle].next_vehicle;
    }
    else
    {
        // С последнего вагона переключаемся на первый
        int cur_train_id = vehicles[cur_vehicle].train_id;
        cur_vehicle = update_data[new_state].trains[cur_train_id].first_vehicle_id;
    }
    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::selectControlVehicle()
{
    // Берём контроль над данным вагоном
    controlled_vehicle = cur_vehicle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehiclesHandler::returnToControlledVehicle()
{
    int prev_cur_vehicle = cur_vehicle;

    // Возврат к управляемому вагону
    cur_vehicle = controlled_vehicle;
    return (cur_vehicle != prev_cur_vehicle);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::load(simulator_vehicles_info_t vehicles_info, const settings_t &settings, vsg::ref_ptr<vsg::Options> options)
{
    int count = vehicles_info.vehicles.size();

    for (int i = 0; i < count; ++i)
    {
        QString cfg_dir_tmp = vehicles_info.vehicles[i].vehicle_config_dir;
        std::string cfg_dir = cfg_dir_tmp.toStdString();

        QString cfg_file_tmp = vehicles_info.vehicles[i].vehicle_config_file;
        std::string cfg_file = cfg_file_tmp.toStdString();

        VehicleExterior vehicle_ext;
        vehicle_ext.driver_pos = settings.cabine_default_pos;
        vehicle_ext.saved_cabine_cam_fov = settings.fovy;

        if (vehicle_ext.loadVehicle(cfg_dir, cfg_file, sound_manager, options))
        {
            LOG_INFO("Loaded vehicle model from %s / %s .xml", cfg_dir.c_str(), cfg_file.c_str());
            LOG_INFO("Vehicle %u / %u loaded", i + 1, count);
        }
        else
        {
            LOG_WARN("Fail to load vehicle model from %s / %s .xml", cfg_dir.c_str(), cfg_file.c_str());
            LOG_WARN("Vehicle %u / %u added with empty model", i + 1, count);
        }

        vehicles.push_back(vehicle_ext);
        vehicles_node->addChild(vehicle_ext.transform);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::slotGetVehiclesPosData(QByteArray &data)
{
    if (unused_data < 0)
    {
        is_pos_updated = false;

        if (new_data < 0)
        {
            // Первое получение данных
            new_data = DATA_ARRAY_SIZE - 3;
            update_pos_data[new_data].deserialize(data);
            if (update_pos_data[new_data].vehicles.size() == vehicles.size())
            {
                time_difference = update_pos_data[new_data].time - ref_time -
                                  settings_delay;
            }
            else
            {
                LOG_WARN("Fail to update: get %u positions but there are %u vehicles",
                         update_pos_data[new_data].vehicles.size(),
                         vehicles.size());
                new_data = -1;
            }
            return;
        }

        if (delay_data < 0)
        {
            // Второе получение данных
            delay_data = new_data;
            new_data = DATA_ARRAY_SIZE - 2;
            update_pos_data[new_data].deserialize(data);
            if (update_pos_data[new_data].vehicles.size() == vehicles.size())
            {
                double r = 0.5;
                time_difference = time_difference * (1.0 - r) +
                                  (update_pos_data[new_data].time - ref_time) * r -
                                  settings_delay;
            }
            else
            {
                LOG_WARN("Fail to update: get %u positions but there are %u vehicles",
                         update_pos_data[new_data].vehicles.size(),
                         vehicles.size());
                new_data = -1;
                delay_data = -1;
            }
            return;
        }

        // Третье получение данных
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
                              (update_pos_data[new_data].time - ref_time) * r -
                              settings_delay;
        }
        else
        {
            LOG_WARN("Fail to update: get %u positions but there are %u vehicles",
                     update_pos_data[new_data].vehicles.size(),
                     vehicles.size());
            new_data = -1;
            delay_data = -1;
        }
        return;
    }

    // Обновление данных по очереди
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
                          (update_pos_data[new_data].time - ref_time) * r -
                          settings_delay;
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
void VehiclesHandler::slotGetVehiclesStateData(QByteArray &data)
{
    update_data[unused_state].deserialize(data);
    if (update_data[unused_state].vehicles.size() == vehicles.size())
    {
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
void VehiclesHandler::slotGetVehicleControlled(QByteArray &data)
{
// deserialize and update debug interface here?
}
