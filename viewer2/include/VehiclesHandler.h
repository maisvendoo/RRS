#ifndef VEHICLES_HANDLER_H
#define VEHICLES_HANDLER_H

#include <QObject>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

#include "simulator-info-struct.h"
#include "simulator-update-struct.h"
#include "VehicleExterior.h"

struct settings_t;
class SoundManager;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VehiclesHandler : public QObject
{
    Q_OBJECT

public:

    VehiclesHandler(const settings_t& settings, SoundManager *sm, QObject* parent = Q_NULLPTR);

    /// Get scene group
    vsg::ref_ptr<vsg::Group> getExterior();

    /// Info about current vehicle exterior
    VehicleExterior *getCurrentVehicle();
    int getCurrentVehicleIndex();
    int getControlledVehicleIndex();

    bool isUpdated();

    QString getDebugMsg();

    void step(double t, double dt);

    bool selectNextTrain();
    bool selectPrevTrain();
    bool selectNextVehicle();
    bool selectPrevVehicle();

    void selectControlVehicle();
    bool returnToControlledVehicle();

    void load(simulator_vehicles_info_t vehicles_info, const settings_t& settings, vsg::ref_ptr<vsg::Options> options);

public slots:

    void slotGetVehiclesPosData(QByteArray &data);
    void slotGetVehiclesStateData(QByteArray &data);
    void slotGetVehicleControlled(QByteArray &data);

signals:

    void updated();

private:

    /// Sound manager
    SoundManager *sound_manager;

    enum {DATA_ARRAY_SIZE = 5};
    /// Data about vehicles positions, received from server
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

    enum {STATE_ARRAY_SIZE = 2};
    /// Data about trains and vehicles state, received from server
    std::array<simulator_update_t, STATE_ARRAY_SIZE> update_data;
    bool is_state_updated = false;
    short new_state = 0;
    short unused_state = 1;
    bool is_new_state = false;

    /// Debug strings for controlled and current vehicles
    simulator_vehicle_controlled_update_t vehicle_controlled;

    /// Updated status
    bool is_updated = false;

    /// Vehicle number which is a referenced for camera
    int cur_vehicle = 0;

    /// Vehicle number which is contorolled by user
    int controlled_vehicle = 0;

    /// Debug message for current and controlled vehicles from server
    QString debug_msg = "";

    /// Train exterior scene group
    vsg::ref_ptr<vsg::Group> vehicles_node = vsg::Group::create();

    /// Info about vehicles exterior
    std::vector<VehicleExterior> vehicles;

    void updateDebugString();
};

#endif // VEHICLES_HANDLER_H
