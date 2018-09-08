#include    "train.h"

#include    "CfgReader.h"
#include    "physics.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Train::Train(FileSystem *fs, QObject *parent) : OdeSystem(parent)
  , fs(fs)
  , trainMass(0.0)
  , trainLength(0.0)
  , ode_order(0)
  , dir(1)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Train::~Train()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Train::init(const init_data_t &init_data)
{
    solver_config = init_data.solver_config;

    // Solver loading
    QString solver_path = fs->getLibDirectory() + solver_config.method;

    train_motion_solver = loadSolver(solver_path);

    if (train_motion_solver == Q_NULLPTR)
    {
        emit logMessage("ERROR: solver " + solver_config.method + " is't found");
        return false;
    }

    QString full_config_path = fs->getTrainsDirectory() +
            init_data.train_config_path + ".xml";

    // Loading of train
    if (!loadTrain(full_config_path))
    {
        emit logMessage("ERROR: train is't loaded");
        return false;
    }

    // State vector initialization
    y.resize(ode_order);
    dydt.resize(ode_order);

    for (size_t i = 0; i < y.size(); i++)
        y[i] = dydt[i] = 0;

    // Loading of couplings
    if (!loadCouplings(full_config_path))
    {
        emit logMessage("ERROR: couplings is't loaded");
        return false;
    }

    // Set initial conditions
    setInitConditions(init_data);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::calcDerivative(state_vector_t &Y, state_vector_t &dYdt, double t)
{
    size_t num_vehicles = vehicles.size();
    auto end = vehicles.end();
    auto coup_it = couplings.begin();

    for (auto it = vehicles.begin(); it != end; ++it)
    {
        Vehicle *vehicle = *it;
        int idx = vehicle->getIndex();
        int s = vehicle->getDegressOfFreedom();

        if ( (num_vehicles > 1) && ( it != end - 1) )
        {
            Vehicle *vehicle1 = *(it+1);
            int idx1 = vehicle1->getIndex();
            int s1 = vehicle1->getDegressOfFreedom();

            double ds = Y[idx] - Y[idx1] -
                    dir * vehicle->getLength() / 2 -
                    dir * vehicle1->getLength() / 2;

            double dv = Y[idx + s] - Y[idx1 + s1];

            Coupling *coup = *coup_it;
            double R = coup->getForce(ds, dv);
            ++coup_it;

            vehicle->setBackwardForce(R);
            vehicle1->setForwardForce(R);
        }

        vehicle->setInclination(0.0);
        vehicle->setCurvature(0.0);

        state_vector_t a = vehicle->getAcceleration(Y, t);

        memcpy(dYdt.data() + idx, Y.data() + idx + s, sizeof(double) * s);
        memcpy(dYdt.data() + idx + s, a.data(), sizeof(double) * s);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::preStep(double t)
{
    (void) t;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Train::step(double t, double &dt)
{
    // EDIT THIS SOLVER CALL. CONFIGURE SOLVER!!!!
    bool done = train_motion_solver->step(this, y, dydt, t, dt,
                                          solver_config.max_step,
                                          solver_config.local_error);    

    return done;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::vehiclesStep(double t, double dt)
{
    auto end = vehicles.end();

    for (auto i = vehicles.begin(); i != end; ++i)
    {
        Vehicle *vehicle = *i;
        vehicle->integrationStep(y, t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::postStep(double t)
{
    (void) t;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Vehicle *Train::getFirstVehicle() const
{
    return *vehicles.begin();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Vehicle *Train::getLastVehicle() const
{
    return *(vehicles.end() - 1);
}

double Train::getVelocity(size_t i) const
{
    if (i < vehicles.size())
    {
        int idx = vehicles[i]->getIndex();
        int s = vehicles[i]->getDegressOfFreedom();
        return y[idx + s];
    }

    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Train::loadTrain(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QDomNode vehicle_node = cfg.getFirstSection("Vehicle");

        size_t index = 0;

        while (!vehicle_node.isNull())
        {
            QString module_name = "";
            if (!cfg.getString(vehicle_node, "Module", module_name))
            {
                emit logMessage("ERROR: Module section is not find");
                break;
            }

            QString module_cfg_name = "";
            if (!cfg.getString(vehicle_node, "ModuleConfig", module_cfg_name))
            {
                break;
            }

            // Calculate module library path
            QString relModulePath = fs->combinePath(module_name, module_name);

            // Vehicles count
            int n_vehicles = 0;
            if (!cfg.getInt(vehicle_node, "Count", n_vehicles))
            {
                n_vehicles = 0;
            }

            // Payload coefficient of vehicles group
            double payload_coeff = 0;
            if (!cfg.getDouble(vehicle_node, "PayloadCoeff", payload_coeff))
            {
                payload_coeff = 0;
            }

            for (int i = 0; i < n_vehicles; i++)
            {
                Vehicle *vehicle = loadVehicle(fs->getModulesDirectory() +
                                               relModulePath);

                if (vehicle == Q_NULLPTR)
                {
                    emit logMessage("ERROR: vehicle " + module_name + " is't loaded");
                    break;
                }

                connect(vehicle, &Vehicle::logMessage, this, &Train::logMessage);

                QString relConfigPath = fs->combinePath(module_name, module_cfg_name);
                vehicle->init(fs->getVehiclesDirectory() + relConfigPath + ".xml");

                vehicle->setPayloadCoeff(payload_coeff);

                trainMass += vehicle->getMass();
                trainLength += vehicle->getLength();

                size_t s = vehicle->getDegressOfFreedom();

                ode_order += 2 * s;

                vehicle->setIndex(index);
                index = ode_order;

                vehicles.push_back(vehicle);
            }

            vehicle_node = cfg.getNextSection();
        }
    }
    else
    {
        emit logMessage("ERROR: file " + cfg_path + " is't found");
    }

    // Check train is't empty and return
    return vehicles.size() != 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Train::loadCouplings(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString coupling_module = "";
        if (!cfg.getString("Common", "CouplingModule", coupling_module))
        {
            coupling_module = "default-coupling";
        }

        int num_couplings = static_cast<int>(vehicles.size() - 1);

        for (int i = 0; i < num_couplings; i++)
        {
            Coupling *coupling = loadCoupling(fs->getModulesDirectory() +
                                              coupling_module);

            if (coupling == Q_NULLPTR)
            {
                emit logMessage("ERROR: coupling module " + coupling_module + " is't found");
                return false;
            }

            coupling->loadConfiguration(fs->getCouplingsDirectory() +
                                        coupling_module + ".xml");

            coupling->reset();

            couplings.push_back(coupling);
        }
    }
    else
    {
        emit logMessage("ERROR: file " + cfg_path + " is't found");
    }

    return couplings.size() != 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::setInitConditions(const init_data_t &init_data)
{
    for (size_t i = 0; i < vehicles.size(); i++)
    {
        Vehicle *vehicle = vehicles[i];

        size_t s = vehicle->getDegressOfFreedom();
        size_t idx = vehicle->getIndex();

        y[idx + s] = init_data.init_velocity / Physics::kmh;

        double wheel_radius = vehicle->getWheelDiameter() / 2.0;

        for (size_t j = 1; j < static_cast<size_t>(s); j++)
        {
            y[idx + s + j] = y[idx + s] / wheel_radius;
        }
    }

    double x0 = init_data.init_coord * 1000.0;
    y[0] = x0;
    dir = init_data.direction;

    for (size_t i = 1; i < vehicles.size(); i++)
    {
        double Li_1 = vehicles[i-1]->getLength();
        size_t idxi_1 = vehicles[i-1]->getIndex();

        double Li = vehicles[i]->getLength();
        size_t idxi = vehicles[i]->getIndex();

        y[idxi] = y[idxi_1] - dir *(Li + Li_1) / 2;
    }
}
