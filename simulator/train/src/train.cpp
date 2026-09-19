#include    "train.h"

#include    "filesystem.h"
#include    "CfgReader.h"
#include    "physics.h"
#include    "Journal.h"
#include    <vehicle-controller.h>
#include    <core/load_module.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Train::Train(QObject *parent) : OdeSystem(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Train::~Train()
{
    delete train_motion_solver;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Train::init(const init_data_t& init_data, int model_vehicles_count)
{
    solver_config = init_data.solver_config;

    coeff_to_wheel_rail_friction = init_data.coeff_to_wheel_rail_friction;

    // Solver loading
    FileSystem &fs = FileSystem::getInstance();
    QString solver_path = QString(fs.getLibraryDir().c_str()) + fs.separator() + solver_config.method;

    train_motion_solver = LOAD_MODULE(Solver, solver_path);

    if (train_motion_solver == nullptr)
    {
        Journal::instance()->error("Solver " + solver_path + " is't found");
        return false;
    }

    Journal::instance()->info(QString("Created Solver object at address: 0x%1; loaded from: %2")
                                  .arg(reinterpret_cast<quint64>(train_motion_solver), 0, 16)
                                  .arg(solver_path));

    QString full_config_path = QString(fs.getTrainsDir().c_str()) +
            fs.separator() +
            init_data.train_config + ".xml";

    Journal::instance()->info("Train config from file: " + full_config_path);

    // Loading of train
    if (!loadTrain(full_config_path, init_data, model_vehicles_count))
    {
        Journal::instance()->error("Train is't loaded");
        return false;
    }

    Journal::instance()->info("==== State vector ====");

    // State vector initialization
    y.resize(ode_order);
    dydt.resize(ode_order);

    Journal::instance()->info(QString("Allocated memory for %1 ODE's").arg(ode_order));

    Journal::instance()->info(QString("State vector address: 0x%1")
                              .arg(reinterpret_cast<quint64>(y.data()), 0, 16));

    Journal::instance()->info(QString("State vector derivative address: 0x%1")
                              .arg(reinterpret_cast<quint64>(dydt.data()), 0, 16));

    for (size_t i = 0; i < y.size(); i++)
        y[i] = dydt[i] = 0;

    // Loading of joints
    if (!loadTrainJoints())
    {
        Journal::instance()->error("Joints aren't loaded");
        return false;
    }

    // Set initial conditions
    Journal::instance()->info("==== Initial conditions ====");
    setInitConditions(init_data);

    initVehiclesBrakes();

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Train::init(const solver_config_t& solver_config, std::vector<Vehicle*>& vehicles, state_vector_t& state_vector, std::vector<std::vector<Joint*>>& joints_list)
{
    this->solver_config = solver_config;

    // Solver loading
    FileSystem &fs = FileSystem::getInstance();
    QString solver_path = QString(fs.getLibraryDir().c_str()) + fs.separator() + solver_config.method;

    train_motion_solver = LOAD_MODULE(Solver, solver_path);

    if (train_motion_solver == nullptr)
    {
        Journal::instance()->error("Solver " + solver_path + " is't found");
        return false;
    }

    Journal::instance()->info(QString("Created Solver object at address: 0x%1; loaded from: %2")
                                  .arg(reinterpret_cast<quint64>(train_motion_solver), 0, 16)
                                  .arg(solver_path));

    this->vehicles = vehicles;
    this->y = state_vector;
    this->joints_list = joints_list;

    for (auto vehicle : this->vehicles)
    {
        trainMass += vehicle->getMass();
        trainLength += vehicle->getLength();

        vehicle->setStateIndex(ode_order);
        ode_order += 2 * vehicle->getDegressOfFreedom();
    }
    dydt.resize(ode_order);

    Journal::instance()->info(QString("New uncoupled train! Address: 0x%1; size of vehicles %2, joints %3, state_vector %4")
                                  .arg(reinterpret_cast<quint64>(this), 0, 16)
                                  .arg(this->vehicles.size(), 4)
                                  .arg(this->joints_list.size(), 4)
                                  .arg(y.size(), 4));
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::couple(double current_distance, bool is_coupling_to_head, bool is_other_coupled_by_head, Train* other_train)
{
    if (other_train == nullptr)
        return;

    // ПЕ поезда, с которым сцепляемся
    std::vector<Vehicle*> other_vehicles = *(other_train->getVehicles());

    // Вектор состояния поезда, с которым сцепляемся
    state_vector_t other_y = other_train->getStateVector();
    double other_begin = other_y[0];
    std::vector<double> other_veh_distances;
    for (size_t i = 1; i < other_vehicles.size(); ++i)
    {
        size_t other_idx = other_vehicles[i]->getStateIndex();
        double other_coord = other_y[other_idx];
        other_veh_distances.push_back(abs(other_coord - other_begin));
        other_begin = other_coord;
    }

    // Массив межвагонных связей поезда, с которым сцепляемся
    std::vector<std::vector<Joint*>> other_joints_list = other_train->getJoints();

    Vehicle* veh;
    device_list_t* cons;
    Vehicle* other_veh;
    device_list_t* other_cons;

    std::vector<Vehicle*> new_vehicles;
    std::vector<std::vector<Joint*>> new_joints_list;
    state_vector_t new_y;
    size_t new_ode_order = 0;

    auto create_joints = [](Train* t,
                            Vehicle* veh, device_list_t* cons,
                            Vehicle* other_veh, device_list_t* other_cons)
        -> std::vector<Joint *>
    {
        std::vector<Joint *> joints;
        if ((cons->empty()) || (other_cons->empty()))
        {
            Journal::instance()->warning(QString("#%1 or #%2 have no connectors. Created empty array of joints.")
                                             .arg(veh->getModelIndex())
                                             .arg(other_veh->getModelIndex()));
        }
        else
        {
            t->loadJoints(cons, other_cons, joints);

            if (joints.empty())
            {
                Journal::instance()->warning(QString("No joints beetween #%1 and #%2. Created empty array of joints.")
                                                 .arg(veh->getModelIndex())
                                                 .arg(other_veh->getModelIndex()));
            }
            else
            {
                Journal::instance()->info(QString("Created %1 joints beetween #%2 and #%3")
                                              .arg(joints.size())
                                              .arg(veh->getModelIndex())
                                              .arg(other_veh->getModelIndex()));
            }
        }
        return joints;
    };

    // Соединяем ПЕ в общий массив
    if (is_coupling_to_head)
    {
        veh = *(vehicles.begin());
        cons = (veh->getDirection() == -1) ?
                                  veh->getBwdConnectors() :
                                  veh->getFwdConnectors();

        if (is_other_coupled_by_head)
        {
            other_veh = *(other_vehicles.begin());
            other_cons = (other_veh->getDirection() == -1) ?
                             other_veh->getBwdConnectors() :
                             other_veh->getFwdConnectors();

            (veh->getDirection() == -1) ?
                veh->setNextVehicle(other_veh) :
                veh->setPrevVehicle(other_veh);
            (other_veh->getDirection() == -1) ?
                other_veh->setNextVehicle(veh) :
                other_veh->setPrevVehicle(veh);

            // Добавляем ПЕ и их вектор состояния в обратном порядке
            for (size_t i = other_vehicles.size(); i > 0; --i)
            {
                Vehicle* vehicle = other_vehicles[i - 1];
                size_t old_idx = vehicle->getStateIndex();
                size_t s = vehicle->getDegressOfFreedom();

                for (size_t j = old_idx; j < old_idx + 2 * s; ++j)
                {
                    new_y.push_back(other_y[j]);
                }
                vehicle->setStateIndex(new_ode_order);
                new_ode_order += 2 * s;

                new_vehicles.push_back(vehicle);
            }

            // Новые поездные координаты для прицепленных ПЕ
            double train_coord = y[0];
            double distance = current_distance + veh->getLength() / 2.0 + other_veh->getLength() / 2.0;
            other_veh_distances.insert(other_veh_distances.begin(), distance);
            for (size_t i = 0; i < other_vehicles.size(); ++i)
            {
                Vehicle* vehicle = other_vehicles[i];
                size_t model_idx = vehicle->getModelIndex();
                size_t idx = vehicle->getStateIndex();

                // На всякий случай актуализируем положение ПЕ в топологии
                // по старой дуговой координате
                auto& vc = topology->getVehicleController(model_idx);
                vc.setPathCoord(vehicle->getDirection() * new_y[idx]);

                vehicle->setDirection(-vehicle->getDirection());

                // Новая дуговая координата
                new_y[idx] = train_coord + other_veh_distances[i];
                vc.setInitPathCoord(vehicle->getDirection() * new_y[idx]);
                train_coord = new_y[idx];

                // Новый индекс поезда
                vc.setTrainIndex(train_idx);
                vehicle->setTrainIndex(train_idx);
            }

            for (size_t i = other_joints_list.size(); i > 0; --i)
            {
                new_joints_list.push_back(other_joints_list[i - 1]);

                for (auto joint : other_joints_list[i - 1])
                {
                    joint->swapDevicesLinks();
                }
            }
        }
        else
        {
            other_veh = *(other_vehicles.end() - 1);
            other_cons = (other_veh->getDirection() == -1) ?
                             other_veh->getFwdConnectors() :
                             other_veh->getBwdConnectors();

            (veh->getDirection() == -1) ?
                veh->setNextVehicle(other_veh) :
                veh->setPrevVehicle(other_veh);
            (other_veh->getDirection() == -1) ?
                other_veh->setPrevVehicle(veh) :
                other_veh->setNextVehicle(veh);

            // Добавляем ПЕ и их вектор состояния
            new_vehicles = other_vehicles;
            new_y = other_y;
            new_ode_order = other_y.size();

            // Новые поездные координаты для прицепленных ПЕ
            double train_coord = y[0];
            double distance = current_distance + veh->getLength() / 2.0 + other_veh->getLength() / 2.0;
            other_veh_distances.push_back(distance);
            for (size_t i = other_vehicles.size(); i > 0; --i)
            {
                Vehicle* vehicle = other_vehicles[i - 1];
                size_t model_idx = vehicle->getModelIndex();
                size_t idx = vehicle->getStateIndex();

                // На всякий случай актуализируем положение ПЕ в топологии
                // по старой дуговой координате
                auto& vc = topology->getVehicleController(model_idx);
                vc.setPathCoord(vehicle->getDirection() * new_y[idx]);

                // Новая дуговая координата
                new_y[idx] = train_coord + other_veh_distances[i - 1];
                vc.setInitPathCoord(vehicle->getDirection() * new_y[idx]);
                train_coord = new_y[idx];

                // Новый индекс поезда
                vc.setTrainIndex(train_idx);
                vehicle->setTrainIndex(train_idx);
            }

            new_joints_list = other_joints_list;
        }

        // Создаём новый массив межвагонных связей между крайними ПЕ сцепляемых поездов
        new_joints_list.push_back(create_joints(this, other_veh, other_cons, veh, cons));

        // Задаём для ПЕ данного поезда новые индексы в векторе состояния
        for (size_t i = 0; i < vehicles.size(); ++i)
        {
            Vehicle* vehicle = vehicles[i];

            size_t new_idx = vehicle->getStateIndex() + new_ode_order;
            vehicle->setStateIndex(new_idx);
        }

        vehicles.insert(vehicles.begin(), new_vehicles.begin(), new_vehicles.end());
        joints_list.insert(joints_list.begin(), new_joints_list.begin(), new_joints_list.end());
        y.insert(y.begin(), new_y.begin(), new_y.end());
    }
    else
    {
        veh = *(vehicles.end() - 1);
        cons = (veh->getDirection() == -1) ?
                                  veh->getFwdConnectors() :
                                  veh->getBwdConnectors();

        if (is_other_coupled_by_head)
        {
            other_veh = *(other_vehicles.begin());
            other_cons = (other_veh->getDirection() == -1) ?
                             other_veh->getBwdConnectors() :
                             other_veh->getFwdConnectors();

            (veh->getDirection() == -1) ?
                veh->setPrevVehicle(other_veh) :
                veh->setNextVehicle(other_veh);
            (other_veh->getDirection() == -1) ?
                other_veh->setNextVehicle(veh) :
                other_veh->setPrevVehicle(veh);

            // Добавляем ПЕ и их вектор состояния
            new_vehicles = other_vehicles;
            new_y = other_y;

            // Новые поездные координаты для прицепленных ПЕ
            double train_coord = y[veh->getStateIndex()];
            double distance = current_distance + veh->getLength() / 2.0 + other_veh->getLength() / 2.0;
            other_veh_distances.insert(other_veh_distances.begin(), distance);
            for (size_t i = 0; i < other_vehicles.size(); ++i)
            {
                Vehicle* vehicle = other_vehicles[i];
                size_t model_idx = vehicle->getModelIndex();
                size_t idx = vehicle->getStateIndex();
                vehicle->setStateIndex(idx + y.size());

                // На всякий случай актуализируем положение ПЕ в топологии
                // по старой дуговой координате
                auto& vc = topology->getVehicleController(model_idx);
                vc.setPathCoord(vehicle->getDirection() * new_y[idx]);

                // Новая дуговая координата
                new_y[idx] = train_coord - other_veh_distances[i];
                vc.setInitPathCoord(vehicle->getDirection() * new_y[idx]);
                train_coord = new_y[idx];

                // Новый индекс поезда
                vc.setTrainIndex(train_idx);
                vehicle->setTrainIndex(train_idx);
            }

            new_joints_list = other_joints_list;
        }
        else
        {
            other_veh = *(other_vehicles.end() - 1);
            other_cons = (other_veh->getDirection() == -1) ?
                             other_veh->getFwdConnectors() :
                             other_veh->getBwdConnectors();

            (veh->getDirection() == -1) ?
                veh->setPrevVehicle(other_veh) :
                veh->setNextVehicle(other_veh);
            (other_veh->getDirection() == -1) ?
                other_veh->setPrevVehicle(veh) :
                other_veh->setNextVehicle(veh);

            // Новые поездные координаты для прицепленных ПЕ
            double train_coord = y[veh->getStateIndex()];
            double distance = current_distance + veh->getLength() / 2.0 + other_veh->getLength() / 2.0;
            other_veh_distances.push_back(distance);
            for (size_t i = other_vehicles.size(); i > 0; --i)
            {
                Vehicle* vehicle = other_vehicles[i - 1];
                size_t model_idx = vehicle->getModelIndex();
                size_t idx = vehicle->getStateIndex();
                size_t s = vehicle->getDegressOfFreedom();

                // Добавляем ПЕ и их вектор состояния в обратном порядке
                for (size_t j = idx; j < idx + 2 * s; ++j)
                {
                    new_y.push_back(other_y[j]);
                }
                new_vehicles.push_back(vehicle);

                // На всякий случай актуализируем положение ПЕ в топологии
                // по старой дуговой координате
                auto& vc = topology->getVehicleController(model_idx);
                vc.setPathCoord(vehicle->getDirection() * other_y[idx]);

                vehicle->setDirection(-vehicle->getDirection());

                // Новая дуговая координата
                new_y[new_ode_order] = train_coord - other_veh_distances[i - 1];
                vc.setInitPathCoord(vehicle->getDirection() * new_y[new_ode_order]);
                train_coord = new_y[new_ode_order];

                vehicle->setStateIndex(new_ode_order + y.size());
                new_ode_order += 2 * s;

                // Новый индекс поезда
                vc.setTrainIndex(train_idx);
                vehicle->setTrainIndex(train_idx);
            }

            for (size_t i = other_joints_list.size(); i > 0; --i)
            {
                new_joints_list.push_back(other_joints_list[i - 1]);

                for (auto joint : other_joints_list[i - 1])
                {
                    joint->swapDevicesLinks();
                }
            }
        }

        // Создаём новый массив межвагонных связей между крайними ПЕ сцепляемых поездов
        joints_list.push_back(create_joints(this, veh, cons, other_veh, other_cons));

        vehicles.insert(vehicles.end(), new_vehicles.begin(), new_vehicles.end());
        joints_list.insert(joints_list.end(), new_joints_list.begin(), new_joints_list.end());
        y.insert(y.end(), new_y.begin(), new_y.end());
    }

    trainMass += other_train->getMass();
    trainLength += other_train->getLength();

    ode_order = y.size();
    train_motion_solver->setODEsize(ode_order);
    dydt.resize(ode_order);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Train* Train::uncouple(double uncoupling_distance)
{
    if (vehicles.size() < 2)
        return nullptr;

    double prev_coord = y[0];
    double prev_length_half = vehicles[0]->getLength() / 2.0;
    for (size_t i = 1; i < vehicles.size(); ++i)
    {
        size_t idx = vehicles[i]->getStateIndex();
        double coord = y[idx];
        double length_half = vehicles[i]->getLength() / 2.0;
        double distance = abs(prev_coord - coord) - length_half - prev_length_half;
        prev_coord = coord;
        prev_length_half = length_half;

        if (distance < uncoupling_distance)
            continue;

        Journal::instance()->info(QString("Train #%1 will be uncoupled between its vehicles %2 (#%3) and %4 (#%5) at distance %6 m")
                                      .arg(train_idx, 3)
                                      .arg(i - 1, 3)
                                      .arg(vehicles[i - 1]->getModelIndex(), 4)
                                      .arg(i, 3)
                                      .arg(vehicles[i]->getModelIndex(), 4)
                                      .arg(distance, 7, 'f', 3));

        Train* new_train = new Train();
        new_train->setTopology(topology);

        (vehicles[i - 1]->getDirection() == -1) ?
            vehicles[i - 1]->setPrevVehicle(nullptr) :
            vehicles[i - 1]->setNextVehicle(nullptr);
        (vehicles[i]->getDirection() == -1) ?
            vehicles[i]->setNextVehicle(nullptr) :
            vehicles[i]->setPrevVehicle(nullptr);

        std::vector<Vehicle*> new_vehicles;
        for (size_t j = i; j < vehicles.size(); ++j)
        {
            new_vehicles.push_back(vehicles[j]);
        }

        state_vector_t new_y;
        for (size_t j = idx; j < y.size(); ++j)
        {
            new_y.push_back(y[j]);
        }

        std::vector<std::vector<Joint*>> new_joints_list;
        if (i < vehicles.size() - 1)
        {
            for (size_t j = i; j < joints_list.size(); ++j)
            {
                new_joints_list.push_back(joints_list[j]);
            }
        }

        vehicles.resize(i);

        trainMass = 0.0;
        trainLength = 0.0;
        for (auto vehicle : vehicles)
        {
            trainMass += vehicle->getMass();
            trainLength += vehicle->getLength();
        }

        ode_order = idx;
        y.resize(ode_order);
        dydt.resize(ode_order);
        train_motion_solver->setODEsize(ode_order);

        for (auto joint : joints_list[i - 1])
        {
            delete joint;
        }
        joints_list.resize(i - 1);

        // ОТЛАДКА
        Journal::instance()->info(QString("Trains uncoupled! Train #%1: new size of vehicles %2, joints %3, state_vector %4")
                                      .arg(train_idx, 3)
                                      .arg(vehicles.size(), 4)
                                      .arg(joints_list.size(), 4)
                                      .arg(y.size(), 4));

        if (new_train->init(solver_config, new_vehicles, new_y, new_joints_list))
            return new_train;
        return nullptr;
    }
    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::reverse()
{
    // Поезд из единственной ПЕ - примитивный разворот ориентации и движения
    if (vehicles.size() == 1)
    {
        size_t s = vehicles[0]->getDegressOfFreedom();
        vehicles[0]->setDirection(-vehicles[0]->getDirection());
        y[0] = -y[0];
        y[s] = -y[s];
        return;
    }

    // Заготовка под разворот вектора состояния
    state_vector_t new_y;
    new_y.reserve(y.size());

    // Проходим по ПЕ, от хвоста к голове
    for (size_t i = vehicles.size(); i > 0; --i)
    {
        // Текущий индекс и количество величин (степеней свободы) в векторе состояния
        Vehicle* vehicle = vehicles[i - 1];
        size_t idx = vehicle->getStateIndex();
        size_t s = vehicle->getDegressOfFreedom();

        // Разворачиваем ориентацию
        vehicle->setDirection(-vehicle->getDirection());
        // Новый индекс в векторе состояния после разворота
        vehicle->setStateIndex(new_y.size());

        // Добавляем величины этой ПЕ в новый вектор состояния
        // Координату и скорость берём с противоположным знаком
        new_y.push_back(-y[idx]);
        for (size_t j = idx + 1; j < idx + s; ++j)
        {
            new_y.push_back(y[j]);
        }
        new_y.push_back(-y[idx + s]);
        for (size_t j = idx + s + 1; j < idx + 2 * s; ++j)
        {
            new_y.push_back(y[j]);
        }
    }
    // Разворачиваем межвагонные связи
    for (size_t i = joints_list.size(); i > 0; --i)
    {
        for (auto joint : joints_list[i - 1])
        {
            joint->swapDevicesLinks();
        }
    }

    // Присваиваем новый вектор состояния
    y.swap(new_y);
    // Разворачиваем порядок хранения указателей на ПЕ и межвагонные связи
    std::reverse(vehicles.begin(), vehicles.end());
    std::reverse(joints_list.begin(), joints_list.end());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::setDistanceToEndOfTrajectory(bool is_train_head, double distance)
{
    if (is_train_head)
        distance_to_stop_head = distance;
    else
        distance_to_stop_tail = distance;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::setTrainIndex(size_t idx)
{
    train_idx = idx;
    for (auto vehicle : vehicles)
    {
        vehicle->setTrainIndex(idx);

        size_t model_idx = vehicle->getModelIndex();
        VehicleController& vc = topology->getVehicleController(model_idx);
        vc.setTrainIndex(idx);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t Train::getTrainIndex() const
{
    return train_idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::calcDerivative(state_vector_t &Y, state_vector_t &dYdt, double t, double dt)
{
    auto begin = vehicles.begin();
    auto end = vehicles.end();

    for (auto it = begin; it != end; ++it)
    {
        Vehicle* vehicle = *it;
        size_t idx = vehicle->getStateIndex();
        size_t s = vehicle->getDegressOfFreedom();

        vehicle->getAcceleration(Y, dYdt, t, dt);

        std::memcpy(dYdt.data() + idx, Y.data() + idx + s, sizeof(double) * s);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Vehicle* Train::getFirstVehicle() const
{
    return *vehicles.begin();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Vehicle* Train::getLastVehicle() const
{
    return *(vehicles.end() - 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
state_vector_t Train::getStateVector()
{
    return y;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<std::vector<Joint*>> Train::getJoints()
{
    return joints_list;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Train::getVelocity(size_t i) const
{
    if (i < vehicles.size())
    {
        size_t idx = vehicles[i]->getStateIndex();
        size_t s = vehicles[i]->getDegressOfFreedom();
        double dir = static_cast<double>(vehicles[i]->getDirection());
        return dir * y[idx + s];
    }
    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Train::getVelocity() const
{
    if (vehicles.size() == 2)
    {
        // Если поезд из двух ПЕ, принимаем за скорость поезда более медленную,
        // так как более быстрая вероятно сейчас отцепляется
        const double v0 = getVelocity(0);
        const double v1 = getVelocity(1);
        if (abs(v0) < abs(v1))
        {
            return v0;
        }
        return v1;
    }

    // Принимаем за скорость поезда скорость ПЕ в середине
    return getVelocity(vehicles.size() / 2);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Train::getMass() const
{
    return trainMass;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Train::getLength() const
{
    return trainLength;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t Train::getVehiclesNumber() const
{
    return vehicles.size();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<Vehicle*>* Train::getVehicles()
{
    return &vehicles;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::setTopology(Topology* topology)
{
    this->topology = topology;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::slotStep(const simulator_time_t& current_time, const double& integration_time)
{
    auto begin = vehicles.begin();
    auto end = vehicles.end();
    Vehicle* first = *begin;
    Vehicle* last = *(end - 1);

    double t = current_time.simulation_seconds;
    double num_sub_step = ceil(integration_time / solver_config.step);
    double dt = integration_time / num_sub_step;
    size_t num_step = static_cast<size_t>(num_sub_step);

    double head_stop_coord;
    double tail_stop_coord;
    bool stop_head = (DISTANCE_TO_COUPLE_TRAINS - distance_to_stop_head > Physics::ZERO);
    bool stop_tail = (DISTANCE_TO_COUPLE_TRAINS - distance_to_stop_tail > Physics::ZERO);
    if (stop_head)
    {
        head_stop_coord = y[first->getStateIndex()] + distance_to_stop_head;
    }
    if (stop_tail)
    {
        tail_stop_coord = y[last->getStateIndex()] - distance_to_stop_tail;
    }

    for (size_t i = 0; i < num_step; ++i)
    {
        // prestep
        for (auto it = begin; it != end; ++it)
        {
            Vehicle* vehicle = *it;

            if (i == 0)
            {
                vehicle->integrationProcess(current_time, integration_time);
            }

            vehicle->setFrictionCoeff(coeff_to_wheel_rail_friction);

            vehicle->integrationPreStep(y, t);
        }

        // step
        auto joints_it = joints_list.begin();
        for (auto it = begin; it != end; ++it)
        {
            if (joints_it != joints_list.end())
            {
                std::vector<Joint*> joints = *joints_it;
                if (!joints.empty())
                {
                    for (auto joint : joints)
                    {
                        joint->step(t, dt);
                    }
                }
                ++joints_it;
            }

            Vehicle* vehicle = *it;
            vehicle->integrationStep(y, t, dt);
        }

        if (stop_head)
        {
            double distance = head_stop_coord - y[first->getStateIndex()];
            double velocity = y[first->getStateIndex() + first->getDegressOfFreedom()];
            double force = calcStopForce(distance, velocity, first->getMass(), dt);
            if (force != 0.0)
            {
                (first->getDirection() == -1) ?
                    first->addBackwardForce(-force) :
                    first->addForwardForce(-force);
            }
        }

        if (stop_tail)
        {
            double distance = y[last->getStateIndex()] - tail_stop_coord;
            double velocity = y[last->getStateIndex() + last->getDegressOfFreedom()];
            double force = calcStopForce(distance, -velocity, last->getMass(), dt);
            if (force != 0.0)
            {
                (last->getDirection() == -1) ?
                    last->addForwardForce(-force) :
                    last->addBackwardForce(-force);
            }
        }

        // solver step
        train_motion_solver->step(this, y, dydt, t, dt,
                                  solver_config.max_step,
                                  solver_config.local_error);
        t += dt;

        // poststep
        for (auto it = begin; it != end; ++it)
        {
            Vehicle* vehicle = *it;
            vehicle->integrationPostStep(y, t);

            if (i == num_step - 1)
            {
                size_t model_idx = vehicle->getModelIndex();
                size_t idx = vehicle->getStateIndex();
                VehicleController& vc = topology->getVehicleController(model_idx);
                vc.setPathCoord(vehicle->getDirection() * y[idx]);
                *(vehicle->getProfilePoint()) = vc.getPosition();
            }
        }
    }
    emit stepDone(train_idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Train::loadTrain(QString cfg_path, const init_data_t& init_data, int model_vehicles_count)
{
    CfgReader cfg;
    FileSystem& fs = FileSystem::getInstance();

    Journal::instance()->info("==== Train loading ====");

    if (cfg.load(cfg_path))
    {
        // Get charging pressure and no air flag
        if (!cfg.getDouble("Common", "ChargingPressure", charging_pressure))
        {
            charging_pressure = 0.5;
        }

        if (!cfg.getDouble("Common", "InitMainResPressure", init_main_res_pressure))
        {
            init_main_res_pressure = 0.9;
        }

        if (!cfg.getBool("Common", "NoAir", no_air))
        {
            no_air = false;
        }

        QDomNode vehicle_node = cfg.getFirstSection("Vehicle");

        if (vehicle_node.isNull())
            Journal::instance()->error("There are not Vehicle sections in train config");

        ode_order = 0;

        while (!vehicle_node.isNull())
        {
            Journal::instance()->info("==== Vehicle's group loading ====");

            QString module_lib_name = "";
            if (!cfg.getString(vehicle_node, "Module", module_lib_name))
            {
                Journal::instance()->error("Section with Module library name is not found");
                break;
            }

            QString module_lib_dir = module_lib_name;
            if (!cfg.getString(vehicle_node, "ModuleDir", module_lib_dir))
            {
                Journal::instance()->error("Section with Module directory is not found, using directory with the same name as Module library");
                module_lib_dir = module_lib_name;
            }

            // Calculate module library path
            QString relModulePath = QString(fs.combinePath(module_lib_dir.toStdString(), module_lib_name.toStdString()).c_str());

            QString module_cfg_name = "";
            if (!cfg.getString(vehicle_node, "ModuleConfig", module_cfg_name))
            {
                Journal::instance()->error("Section with Config file name is not found");
                break;
            }

            QString module_cfg_dir = module_cfg_name;
            if (!cfg.getString(vehicle_node, "ModuleConfigDir", module_cfg_dir))
            {
                Journal::instance()->error("Section with Config directory is not found, using directory with the same name as Config file");
                module_cfg_dir = module_cfg_name;
            }

            // Calculate config file path
            QString relConfigPath = QString(fs.combinePath(module_cfg_dir.toStdString(), module_cfg_name.toStdString()).c_str());

            // Vehicles count
            int n_vehicles = 0;
            if (!cfg.getInt(vehicle_node, "Count", n_vehicles))
            {
                n_vehicles = 0;
                Journal::instance()->warning("Count of vehicles " + module_lib_name + " is not found. Vehicle willn't loaded");
            }

            // Orientation of vehicles group
            bool isForward = true;
            if (!cfg.getBool(vehicle_node, "IsOrientationForward", isForward))
            {
                isForward = true;
                Journal::instance()->warning("Orientations of vehicles " + module_lib_name + " is not found.");
            }
            int orient;
            if (isForward)
                orient = 1;
            else
                orient = -1;

            // Payload coefficient of vehicles group
            double payload_coeff = 0;
            if (!cfg.getDouble(vehicle_node, "PayloadCoeff", payload_coeff))
            {
                payload_coeff = 0;
            }

            // Brake shoes state of vehicles group
            bool is_brake_shoes = false;
            if (!cfg.getBool(vehicle_node, "IsBrakeShoes", is_brake_shoes))
            {
                is_brake_shoes = false;
            }

            for (int i = 0; i < n_vehicles; i++)
            {
                Journal::instance()->info("==== Vehicle loading ====");

                Vehicle *vehicle = LOAD_MODULE(Vehicle,
                    QString(fs.getModulesDir().c_str()) +
                    fs.separator() + relModulePath);

                if (vehicle == nullptr)
                {
                    Journal::instance()->error("Vehicle " + module_lib_name + " is't loaded");
                    break;
                }

                Journal::instance()->info(QString("Created Vehicle object at address: 0x%1")
                                          .arg(reinterpret_cast<quint64>(vehicle), 0, 16));

                vehicle->setModuleDir(module_lib_dir);
                vehicle->setModuleName(module_lib_name);
                vehicle->setConfigDir(module_cfg_dir);
                vehicle->setConfigName(module_cfg_name);
                vehicle->setRouteDir(init_data.route_dir_name);

                if (model_vehicles_count >= 0)
                {
                    vehicle->setModelIndex(model_vehicles_count);
                    model_vehicles_count++;
                }
                vehicle->setStateIndex(ode_order);
                vehicle->setPayloadCoeff(payload_coeff);
                vehicle->setBrakeShoesState(is_brake_shoes);
                vehicle->setDirection(orient);

                vehicle->init(QString(fs.getVehiclesDir().c_str()) + fs.separator() + relConfigPath + ".xml");

                trainMass += vehicle->getMass();
                trainLength += vehicle->getLength();

                size_t s = vehicle->getDegressOfFreedom();
                ode_order += 2 * s;

                if (vehicles.size() !=0)
                {
                    Vehicle* prev =  *(vehicles.end() - 1);
                    if (prev->getDirection() > 0)
                        prev->setNextVehicle(vehicle);
                    else
                        prev->setPrevVehicle(vehicle);
                    if (vehicle->getDirection() > 0)
                        vehicle->setPrevVehicle(prev);
                    else
                        vehicle->setNextVehicle(prev);
                }

                vehicles.push_back(vehicle);
            }

            // Advance to next <Vehicle> sibling directly, bypassing
            // CfgReader::curNode which can be corrupted by nested calls.
            QDomNode next = vehicle_node.nextSibling();
            while (!next.isNull() && next.nodeName() != vehicle_node.nodeName())
                next = next.nextSibling();
            vehicle_node = next;
        }
    }
    else
    {
        Journal::instance()->error("File " + cfg_path + " is't found");
    }

    // Check train is't empty and return
    return vehicles.size() != 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Train::loadTrainJoints()
{
    Journal::instance()->info("==== Joints loading ====");

    joints_list.clear();

    size_t num_joints = vehicles.size() - 1;

    if (num_joints == 0)
    {
        Journal::instance()->info("There is only one vehicle! No joints needed");
        return true;
    }

    size_t i = 0;
    auto begin = vehicles.begin();
    auto end = vehicles.end();

    for (auto it = begin; it != end - 1; ++it)
    {
        ++i;

        // Pair of neighbor vehicles
        Vehicle *veh_fwd = *it;
        Vehicle *veh_bwd = *(it+1);

        // Get connectors list from ahead vehicle
        device_list_t* cons_fwd;
        if (veh_fwd->getDirection() > 0)
            cons_fwd = veh_fwd->getBwdConnectors();
        else
            cons_fwd = veh_fwd->getFwdConnectors();

        // Get connectors list from behind vehicle
        device_list_t* cons_bwd;
        if (veh_bwd->getDirection() > 0)
            cons_bwd = veh_bwd->getFwdConnectors();
        else
            cons_bwd = veh_bwd->getBwdConnectors();

        // Create array with joints between these two vehicle
        std::vector<Joint*> joints;

        if ((cons_fwd->empty()) || (cons_bwd->empty()))
        {
            joints_list.push_back(joints);
            Journal::instance()->warning(QString("#%1 or #%2 have no connectors. Created empty array of joints.")
                                      .arg(i - 1)
                                      .arg(i));
            continue;
        }

        loadJoints(cons_fwd, cons_bwd, joints);

        if (joints.empty())
        {
            Journal::instance()->warning(QString("No joints beetween #%1 and #%2. Created empty array of joints.")
                                         .arg(i - 1)
                                         .arg(i));
        }
        else
        {
            Journal::instance()->info(QString("Created %1 joints beetween #%2 and #%3")
                                      .arg(joints.size())
                                      .arg(i - 1)
                                      .arg(i));
        }

        // Add joints array to list of all joints
        joints_list.push_back(joints);
    }

    // Check there are joints for each pair of neighbor vehicles
    return joints_list.size() == num_joints;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::loadJoints(device_list_t* cons_fwd, device_list_t* cons_bwd, std::vector<Joint*>& joints)
{
    // First try link connectors with the same name
    for (auto con_fwd_it = cons_fwd->begin(); con_fwd_it != cons_fwd->end(); ++con_fwd_it)
    {
        Device* con_fwd = *con_fwd_it;
        QString name_fwd = con_fwd->getName();

        for (auto con_bwd_it = cons_bwd->begin(); con_bwd_it != cons_bwd->end(); ++con_bwd_it)
        {
            Device* con_bwd = *con_bwd_it;
            QString name_bwd = con_bwd->getName();

            if (name_fwd == name_bwd)
            {
                loadJointModule(con_fwd, con_bwd, joints);
                break;
            }
        }
    }

    // Try link any connectors
    for (auto con_fwd_it = cons_fwd->begin(); con_fwd_it != cons_fwd->end(); ++con_fwd_it)
    {
        Device* con_fwd = *con_fwd_it;
        if (con_fwd->isLinked())
            continue;

        for (auto con_bwd_it = cons_bwd->begin(); con_bwd_it != cons_bwd->end(); ++con_bwd_it)
        {
            Device* con_bwd = *con_bwd_it;
            if (con_bwd->isLinked())
                continue;

            loadJointModule(con_fwd, con_bwd, joints);

            if (con_fwd->isLinked())
                break;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::loadJointModule(Device* con_fwd, Device* con_bwd, std::vector<Joint*>& joints)
{
    CfgReader cfg;
    FileSystem& fs = FileSystem::getInstance();

    QString name_fwd = con_fwd->getName();
    QString name_bwd = con_bwd->getName();
    QString joint_cfg_name = QString("joint-" + name_fwd + "-" + name_bwd);
//    Journal::instance()->info(QString("Try to load config: " + joint_cfg_name));

    // Check forward connector's custom config directory
    QString cfg_dir = QString(fs.getVehiclesDir().c_str());
    QString fwd_cfg_subdir = con_fwd->getCustomConfigDir();

    QString joint_cfg_path = cfg_dir;
    joint_cfg_path += fs.separator() + fwd_cfg_subdir;
    joint_cfg_path += fs.separator() + joint_cfg_name + ".xml";

    if (!cfg.load(joint_cfg_path))
    {
        // Check backward connector's custom config directory
        QString bwd_cfg_subdir = con_bwd->getCustomConfigDir();

        joint_cfg_path = cfg_dir;
        joint_cfg_path += fs.separator() + bwd_cfg_subdir;
        joint_cfg_path += fs.separator() + joint_cfg_name + ".xml";

        if (!cfg.load(joint_cfg_path))
        {
            // Check default directory of devices configuration files
            joint_cfg_path = QString(fs.getDevicesDir().c_str());
            joint_cfg_path += fs.separator() + joint_cfg_name + ".xml";

            if (!cfg.load(joint_cfg_path))
            {
                return;
            }
        }
    }

    Journal::instance()->info("Loaded file: " + joint_cfg_path);
    QString secName = "Joint";

    QString joint_module_dir;
    if (cfg.getString(secName, "ModuleDir", joint_module_dir))
    {
        joint_module_dir = QString(fs.combinePath(fs.getModulesDir(), joint_module_dir.toStdString()).c_str());
    }
    else
    {
        joint_module_dir = QString(fs.getModulesDir().c_str());
    }

    QString joint_module_name = "";
    cfg.getString(secName, "ModuleName", joint_module_name);

    Joint* joint = LOAD_MODULE(Joint,
        QString(joint_module_dir + fs.separator() + joint_module_name));
    if (joint == nullptr)
        return;

    Journal::instance()->info("Loaded joint model from: " + joint_module_name);

    joint->setLink(con_fwd, 0);
    joint->setLink(con_bwd, 1);
    joint->read_config(joint_cfg_path);

    joints.push_back(joint);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::setInitConditions(const init_data_t& init_data)
{
    for (size_t i = 0; i < vehicles.size(); i++)
    {
        Vehicle* vehicle = vehicles[i];

        size_t s = vehicle->getDegressOfFreedom();
        size_t idx = vehicle->getStateIndex();

        y[idx + s] = init_data.init_velocity / Physics::kmh;
        for (size_t j = 1; j < s; j++)
        {
            double wheel_radius = vehicle->getWheelDiameter(j - 1) / 2.0;
            y[idx + s + j] = y[idx + s] / wheel_radius;
        }
    }

    double x0 = 0.0 - this->getFirstVehicle()->getLength() / 2.0;
    y[0] = x0;
    vehicles[0]->setTrainCoord(x0);
    Journal::instance()->info(QString("Vehicle[%2] coordinate: %1").arg(y[0]).arg(0, 3));

    for (size_t i = 1; i < vehicles.size(); i++)
    {
        double Li_1 = vehicles[i-1]->getLength();
        size_t idxi_1 = vehicles[i-1]->getStateIndex();

        double Li = vehicles[i]->getLength();
        size_t idxi = vehicles[i]->getStateIndex();

        y[idxi] = y[idxi_1] - (Li + Li_1) / 2;
        vehicles[i]->setTrainCoord(y[idxi]);
        Journal::instance()->info(QString("Vehicle[%2] coordinate: %1").arg(y[idxi]).arg(i, 3));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Train::initVehiclesBrakes()
{
    Journal::instance()->info("Initialization of vehicles brake devices...");

    for (size_t i = 0; i < vehicles.size(); ++i)
    {
        if (no_air)
        {
            vehicles[i]->initBrakeDevices(charging_pressure, 0.0, init_main_res_pressure);
        }
        else
        {
            double pBP = charging_pressure;
            vehicles[i]->initBrakeDevices(charging_pressure, pBP, init_main_res_pressure);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Train::calcStopForce(double distance, double veh_velocity, double veh_mass, double dt)
{
    // Если отъехали далеко от тупика - не вмешиваемся
    if (distance > DISTANCE_TO_COUPLE_TRAINS)
    {
        return 0.0;
    }

    // Приближение к тупиковому упору
    if (distance >= 0.0)
    {
        // Для торможения к тупику условно зададим ограничение скорости,
        // пропорциональное расстоянию до тупика
        const double over_velocity = veh_velocity - distance;

        // В движение с меньшей скоростью, в т.ч. обратно от тупика, не вмешиваемся
        if (over_velocity < Physics::ZERO)
        {
            return 0.0;
        }

        // Отталкивание от тупика до некоторой маленькой скорости
        const double revert_velocity = 0.25;

        // Рассчитываем силу, необходимую для снижения скорости ПЕ до ограничения
        const double force = veh_mass * (revert_velocity + over_velocity) / dt;

        // Коэффициент из расстояния от точки начала торможения перед тупиковым упором
        // для плавного возрастания тормозного услилия по мере приближения к тупику
        const double k = 1.0 - distance / DISTANCE_TO_COUPLE_TRAINS;

        // Для мягкости торможения применяем меньшее усилие
        return k * force * 0.25;
    }

    // После проезда тупика стремимся вытолкнуть состав обратно с небольшой скоростью
    const double diff_velocity = veh_velocity - tanh(distance * 0.25) * 4.0;

    // Отталкивание от тупика до некоторой маленькой скорости
    const double revert_velocity = 0.25;

    // Рассчитываем силу, необходимую для изменения скорости ПЕ
    // до скорости выталкивания из тупика
    const double force = veh_mass * (revert_velocity + diff_velocity) / dt;

    // Для поглощения энергии слишком быстрому выталкиванию из тупика
    // сопротивляемся полным усилием
    if (diff_velocity < revert_velocity)
    {
        return force;
    }

    // Для мягкости торможения применяем меньшее усилие
    return force * 0.25;
}
