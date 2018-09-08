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

#include    "filesystem.h"
#include    "init_data.h"
#include    "ode-system.h"
#include    "vehicle.h"
#include    "coupling.h"
#include    "solver.h"
#include    "solver-config.h"

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
    explicit Train(FileSystem *fs, QObject *parent = Q_NULLPTR);
    /// Destructor
    virtual ~Train();

    /// Train initialization
    bool init(const init_data_t &init_data);

    /// Calculation of right part motion ODE's
    void calcDerivative(state_vector_t &Y, state_vector_t &dYdt, double t);

    /// Action before time step
    void preStep(double t);

    /// Integration step
    bool step(double t, double &dt);

    /// Integration step for vehicles ODE's
    void vehiclesStep(double t, double dt);

    /// Action after integration step
    void postStep(double t);

    /// Get first vehicle
    Vehicle *getFirstVehicle() const;

    /// Get last vehicle
    Vehicle *getLastVehicle() const;

    double getVelocity(size_t i) const;

signals:

    void logMessage(QString msg);

private:

    /// Pointer to filesystem object
    FileSystem  *fs;

    /// Train mass
    double          trainMass;
    /// Train length
    double          trainLength;

    /// Order of system ODE motion
    size_t          ode_order;

    /// Direction of motion on railway
    int             dir;

    /// Solver's configuration
    solver_config_t solver_config;

    /// Motion ODE's solver
    Solver      *train_motion_solver;

    /// All train's vehicles
    std::vector<Vehicle *> vehicles;

    /// All train's couplings
    std::vector<Coupling *> couplings;

    /// Train's loading
    bool loadTrain(QString cfg_path);
    /// Couplings loading
    bool loadCouplings(QString cfg_path);

    /// Set initial conditions
    void setInitConditions(const init_data_t &init_data);    
};

#endif // TRAIN_H
