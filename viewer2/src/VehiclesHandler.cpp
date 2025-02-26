#include "VehiclesHandler.h"

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
std::vector<VehicleExterior> *VehiclesHandler::getVehicles()
{
    return &vehicles;
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
    for (auto veh : vehicles)
    {
        veh.step(static_cast<float>(t), static_cast<float>(dt));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::selectNextTrain()
{
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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::selectPrevTrain()
{
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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::selectNextVehicle()
{
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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::selectPrevVehicle()
{
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
void VehiclesHandler::load(simulator_vehicles_info_t vehicles_info)
{
    int count = vehicles_info.vehicles.size();

    for (int i = 0; i < count; ++i)
    {
        LOG_INFO("Vehicle %u / %u loading", i + 1, count);

        QString cfg_dir_tmp = vehicles_info.vehicles[i].vehicle_config_dir;
        std::string cfg_dir = cfg_dir_tmp.toStdString();

        QString cfg_file_tmp = vehicles_info.vehicles[i].vehicle_config_file;
        std::string cfg_file = cfg_file_tmp.toStdString();

        VehicleExterior vehicle_ext;
        if (vehicle_ext.loadVehicle(cfg_dir, cfg_file, sound_manager))
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

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::slotGetVehiclesStateData(QByteArray &data)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehiclesHandler::slotGetVehicleControlled(QByteArray &data)
{

}
