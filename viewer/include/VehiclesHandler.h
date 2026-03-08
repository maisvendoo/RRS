#pragma once
#ifndef VEHICLES_HANDLER_H
#define VEHICLES_HANDLER_H

#include <simulator-info-struct.h>
#include <simulator-update-struct.h>
#include <autopilot-timetable.h>
#include <VehicleExterior.h>

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

#include <QObject>
#include <QString>

#include <array>
#include <functional>
#include <vector>

struct settings_t;
class SoundManager;

class QByteArray;

namespace vsg
{
    class Options;
    class Viewer;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VehiclesHandler final : public QObject
{
    Q_OBJECT

public:
    VehiclesHandler(const settings_t& settings, SoundManager* sound_manager, QObject* parent = nullptr);

    /// Get scene group
    vsg::ref_ptr<vsg::Group> getExterior() const noexcept;

    /// Info about current vehicle exterior
    VehicleExterior* getCurrentVehicle();
    int getCurrentVehicleIndex() const noexcept;
    int getControlledVehicleIndex() const noexcept;
    int getCurrentTrainIndex() const noexcept;

    bool isUpdated() const noexcept;

    simulator_time_t* getDateTime();

    QString getDebugMessage() const noexcept;

    void step(double t, double dt);

    bool selectNextTrain() noexcept;
    bool selectPrevTrain() noexcept;
    bool selectNextVehicle() noexcept;
    bool selectPrevVehicle() noexcept;

    bool selectControlVehicle() noexcept;
    bool returnToControlledVehicle() noexcept;

    bool load(
        QByteArray& data,
        const settings_t& settings,
        vsg::ref_ptr<vsg::Options> options
    );

    void set_camera_pos(const vsg::dvec3* camera_pos) noexcept { this->camera_pos = camera_pos; }

    /// Получить данные о графике движения, если таковые имеются в текущей ПЕ
    autopilot_timetable_t getTimetable()
    {
        autopilot_timetable_t timetable;

        if (update_vehicles[0].vehicles[cur_vehicle].timetableData.size() != 0)
        {
            timetable.deserialize(update_vehicles[0].vehicles[cur_vehicle].timetableData);
        }

        return timetable;
    }

public slots:
    void slotGetTrainsData(QByteArray& data);
    void slotGetVehiclesPosData(QByteArray& data);
    void slotGetVehiclesStateData(QByteArray& data);
    void slotGetVehicleControlled(QByteArray& data);

signals:
    void updated();

private:
    void getVehiclesPosData1(QByteArray& data);
    void getVehiclesPosData2(QByteArray& data);
    void getVehiclesPosData3(QByteArray& data);
    void getVehiclesPosData4(QByteArray& data);

    void updateDebugString();

private:
    SoundManager* sound_manager;
    const vsg::dvec3* camera_pos;

    /// Data about vehicles positions, received from server
    static constexpr int DATA_ARRAY_SIZE = 5;
    std::array<simulator_update_pos_t, DATA_ARRAY_SIZE> update_pos_data;
    bool is_pos_updated = false;
    short new_data = -1;
    short delay_data = -1;
    short cur_data = -1;
    short old_data = -1;
    short unused_data = -1;
    double ref_time = 0.0;
    double time_difference = 0.0;
    double settings_delay = 0.17;

    /// Data about trains, received from server
    simulator_trains_update_t update_trains = simulator_trains_update_t();

    /// Data about vehicles state, received from server
    static constexpr int STATE_ARRAY_SIZE = 2;
    std::array<simulator_vehicles_update_t, STATE_ARRAY_SIZE> update_vehicles;
    bool is_state_updated = false;
    short new_state = 0;
    short unused_state = 1;
    bool is_new_state = false;

    /// Data about vehicles, received from server
    simulator_vehicles_info_t vehicles_info;

    /// Debug strings for controlled and current vehicles
    simulator_vehicle_controlled_update_t vehicle_controlled;

    /// Updated status
    bool is_updated = false;

    /// Vehicle number which is a referenced for camera
    int cur_vehicle = 0;

    /// Vehicle number which is contorolled by user
    int controlled_vehicle = 0;

    /// Debug message for current and controlled vehicles from server
    QString debug_message = "";

    /// Train exterior scene group
    vsg::ref_ptr<vsg::Group> vehicles_node = vsg::Group::create();

    /// Info about vehicles exterior
    std::vector<VehicleExterior> vehicles;

    std::function<void(QByteArray&)> current_get_vehicles_pos_data_function;
};

#endif // VEHICLES_HANDLER_H
