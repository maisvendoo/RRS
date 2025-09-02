//------------------------------------------------------------------------------
//
//      Common train's model dynamics
//      (c) maisvendoo, 04/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Common train's model dynamics
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 04/09/2018
 */

#ifndef     TRAIN_H
#define     TRAIN_H

#include    "global-const.h"
#include    "init_data.h"
#include    "ode-system.h"
#include    "vehicle.h"
#include    "device-list.h"
#include    "device-joint.h"
#include    "solver.h"
#include    "solver-config.h"

#include    <topology.h>

#include    <QByteArray>

#if defined(TRAIN_LIB)
    #define TRAIN_EXPORT    Q_DECL_EXPORT
#else
    #define TRAIN_EXPORT    Q_DECL_IMPORT
#endif

/*!
 * \class
 * \brief Common train model
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TRAIN_EXPORT Train : public OdeSystem
{
    Q_OBJECT

public:

    /// Constructor
    explicit Train(QObject* parent = nullptr);
    /// Destructor
    virtual ~Train();

    /// Train initialization
    bool init(const init_data_t& init_data);

    /// Train initialization
    bool init(const solver_config_t& solver_config, int direction, std::vector<Vehicle*>& vehicles, state_vector_t& state_vector, std::vector<std::vector<Joint*>>& joints_list);

    /// Train coupling
    void couple(double current_distance, bool is_coupling_to_head, bool is_other_coupled_by_head, Train* other_train = nullptr);

    /// Train uncoupling
    Train* uncouple(double uncoupling_distance);

    /// Set distance to stop the train before end of trajectory
    void setDistanceToEndOfTrajectory(int direction, double distance);

    /// Set train index
    void setTrainIndex(size_t idx);

    /// Get train index
    size_t getTrainIndex() const;

    /// Calculation of right part motion ODE's
    void calcDerivative(state_vector_t &Y, state_vector_t &dYdt, double t, double dt);

    /// Get first vehicle
    Vehicle* getFirstVehicle() const;

    /// Get last vehicle
    Vehicle* getLastVehicle() const;

    state_vector_t getStateVector();

    std::vector<std::vector<Joint*>> getJoints();

    double getVelocity(size_t i = 0) const;

    /// Get train mass
    double getMass() const;
    /// Get train length
    double getLength() const;

    size_t getVehiclesNumber() const;

    QString getClientName();

    QString getTrainID();

    int getDirection() const;

    std::vector<Vehicle*>* getVehicles();

    void setTopology(Topology* topology);

public slots:

    /// Integration step
    void slotStep(double current_time, double integration_time);

signals:

    /// Integration step done
    void stepDone(int idx);

private:

    /// Train index
    size_t      train_idx = 0;

    /// Train mass
    double      trainMass = 0.0;
    /// Train length
    double      trainLength = 0.0;

    /// Distance to stop the head of train before end of trajectory
    double      distance_to_stop_head = DISTANCE_TO_COUPLE_TRAINS;
    /// Distance to stop the tail of train before end of trajectory
    double      distance_to_stop_tail = DISTANCE_TO_COUPLE_TRAINS;

    /// Order of system ODE motion
    size_t      ode_order = 0;

    /// Direction of motion on railway
    int         dir = 1;

    /// Coefficient to friction between wheel and rail
    double      coeff_to_wheel_rail_friction = 1.0;

    /// Charging pressure
    double      charging_pressure = 0.0;

    /// No air flag (for empty air system on start)
    bool        no_air = false;

    /// Initial main reservoir pressure
    double      init_main_res_pressure = 0.0;

    /// Motion ODE's solver
    Solver*     train_motion_solver = nullptr;

    /// Имя сетевого клиента для ВЖД
    QString     client_name;

    /// Идентификатор поезда для ВЖД
    QString     train_id;

    /// All train's vehicles
    std::vector<Vehicle*> vehicles;

    /// All joints between neighbor vehicles
    std::vector<std::vector<Joint*>> joints_list;

    /// Solver's configuration
    solver_config_t solver_config;

    Topology* topology = nullptr;

    /// Train's loading
    bool loadTrain(QString cfg_path, const init_data_t &init_data);
    /// Joints loading
    bool loadTrainJoints();
    /// Joints loading
    void loadJoints(device_list_t* cons_fwd, device_list_t* cons_bwd, std::vector<Joint*>& joints);
    /// Joint module loading
    void loadJointModule(Device* con_fwd, Device* con_bwd, std::vector<Joint*>& joints);

    /// Set initial conditions
    void setInitConditions(const init_data_t &init_data);

    /// Initialization of vehicles brakes
    void initVehiclesBrakes();

    /// Set initial conditions
    double calcStopForce(double distance, double veh_velocity, double veh_mass, double dt);
};

#endif // TRAIN_H
