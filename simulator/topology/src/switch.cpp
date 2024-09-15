#include    <switch.h>

#include    <filesystem.h>
#include    <Journal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Switch::Switch(QObject *parent) : Connector(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Switch::~Switch()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory *Switch::getFwdTraj() const
{
    // Если траектория вперёд единственная - делать дальше нечего
    if (fwdMinusTraj == Q_NULLPTR)
    {
        if (fwdPlusTraj == Q_NULLPTR)
        {
            return Q_NULLPTR;
        }
        return fwdPlusTraj;
    }
    if (fwdPlusTraj == Q_NULLPTR)
    {
        return fwdMinusTraj;
    }

    // Стрелка в плюсовом положении
    if (state_fwd > 0)
    {
        return fwdPlusTraj;
    }

    // Стрелка в минусовом положении
    if (state_fwd < 0)
    {
        return fwdMinusTraj;
    }

    return Q_NULLPTR;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory *Switch::getBwdTraj() const
{
    // Если траектория вперёд единственная - делать дальше нечего
    if (bwdMinusTraj == Q_NULLPTR)
    {
        if (bwdPlusTraj == Q_NULLPTR)
        {
            return Q_NULLPTR;
        }
        return bwdPlusTraj;
    }
    if (bwdPlusTraj == Q_NULLPTR)
    {
        return bwdMinusTraj;
    }

    // Стрелка в плюсовом положении
    if (state_bwd > 0)
    {
        return bwdPlusTraj;
    }

    // Стрелка в минусовом положении
    if (state_bwd < 0)
    {
        return bwdMinusTraj;
    }

    return Q_NULLPTR;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::configure(CfgReader &cfg, QDomNode secNode, traj_list_t &traj_list)
{
    Connector::configure(cfg, secNode, traj_list);

    cfg.getInt(secNode, "state_fwd", state_fwd);
    cfg.getInt(secNode, "state_bwd", state_bwd);

    Journal::instance()->info("Connector type: switch");

    QString fwd_minus_name;
    cfg.getString(secNode, "fwdMinusTraj", fwd_minus_name);
    fwdMinusTraj = traj_list.value(fwd_minus_name, Q_NULLPTR);

    QString bwd_minus_name;
    cfg.getString(secNode, "bwdMinusTraj", bwd_minus_name);
    bwdMinusTraj = traj_list.value(bwd_minus_name, Q_NULLPTR);

    QString fwd_plus_name;
    cfg.getString(secNode, "fwdPlusTraj", fwd_plus_name);
    fwdPlusTraj = traj_list.value(fwd_plus_name, Q_NULLPTR);

    QString bwd_plus_name;
    cfg.getString(secNode, "bwdPlusTraj", bwd_plus_name);
    bwdPlusTraj = traj_list.value(bwd_plus_name, Q_NULLPTR);

    size_t inputs_count = 0;
    size_t outputs_count = 0;

    if (bwdMinusTraj != Q_NULLPTR)
    {
        bwdMinusTraj->setFwdConnector(this);
        Journal::instance()->info("Backward minus traj: " + bwdMinusTraj->getName());
        inputs_count++;
    }
    else
    {
        Journal::instance()->info("Backward minus traj: NONE");
    }

    if (bwdPlusTraj != Q_NULLPTR)
    {
        bwdPlusTraj->setFwdConnector(this);
        Journal::instance()->info("Backward plus traj: " + bwdPlusTraj->getName());
        inputs_count++;
    }
    else
    {
        Journal::instance()->info("Backward plus traj: NONE");
    }

    if (fwdMinusTraj != Q_NULLPTR)
    {
        fwdMinusTraj->setBwdConnector(this);
        Journal::instance()->info("Forward minus traj: " + fwdMinusTraj->getName());
        outputs_count++;
    }
    else
    {
        Journal::instance()->info("Forward minus traj: NONE");
    }

    if (fwdPlusTraj != Q_NULLPTR)
    {
        fwdPlusTraj->setBwdConnector(this);
        Journal::instance()->info("Forward plus traj: " + fwdPlusTraj->getName());
        outputs_count++;
    }
    else
    {
        Journal::instance()->info("Forward plus traj: NONE");
    }

    if (inputs_count == 0)
    {
        Journal::instance()->error("Switch " + name + " has't incomming trajectories!!!");
    }
    else
    {
        Journal::instance()->info("Incommnig trajectories: " + QString("%1").arg(inputs_count));
        if (inputs_count == 2)
            state_bwd = 1;
    }

    if (outputs_count == 0)
    {
        Journal::instance()->error("Switch " + name + " has't outgoing trajectories!!!");
    }
    else
    {
        Journal::instance()->info("Outgoing trajectories: " + QString("%1").arg(outputs_count));
        if (outputs_count == 2)
            state_fwd = 1;
    }

    // Загружаем модули
    // Находим названия модулей, которые есть и в траекториях спереди, и в траекториях сзади
    QStringList devices_names;
    for (auto traj_bwd : {bwdPlusTraj, bwdMinusTraj})
    {
        if (traj_bwd == nullptr)
            continue;

        QStringList devices_names_bwd;
        for (auto device_bwd : traj_bwd->getTrajectoryDevices())
        {
            QString name = device_bwd->getName();
            if (!devices_names_bwd.contains(name))
                devices_names_bwd.push_back(name);
        }

        for (auto traj_fwd : {fwdPlusTraj, fwdMinusTraj})
        {
            if (traj_fwd == nullptr)
                continue;

            for (auto device_fwd : traj_bwd->getTrajectoryDevices())
            {
                QString name = device_fwd->getName();
                if (devices_names_bwd.contains(name) && (!devices_names.contains(name)))
                    devices_names.push_back(name);
            }
        }
    }

    if (devices_names.isEmpty())
        return;

    FileSystem &fs = FileSystem::getInstance();
    for (auto device_name : devices_names)
    {
        QString conn_module = "connector-" + device_name;
        QString conn_path = QString(fs.getModulesDir().c_str()) +
                                     QDir::separator() +
                                     conn_module;
        ConnectorDevice *module = loadConnectorDevice(conn_path);

        if (module == nullptr)
        {
            Journal::instance()->error("Module " + conn_module + " for " + name + " not found");
        }
        else
        {
            module->setConnector(this);

            bool no_plus;
            if (bwdPlusTraj != nullptr)
            {
                no_plus = false;
                for (auto device_bwd : bwdPlusTraj->getTrajectoryDevices())
                {
                    QString bwd_name = device_bwd->getName();
                    if (device_name == bwd_name)
                    {
                        module->setBwdTrajectoryDevice(device_bwd);
                        device_bwd->setFwdConnectorDevice(module);
                        break;
                    }
                }
            }
            else
            {
                no_plus = true;
            }

            if (bwdMinusTraj != nullptr)
            {
                for (auto device_bwd : bwdMinusTraj->getTrajectoryDevices())
                {
                    QString bwd_name = device_bwd->getName();
                    if (device_name == bwd_name)
                    {
                        if (no_plus)
                            module->setBwdTrajectoryDevice(device_bwd);
                        device_bwd->setFwdConnectorDevice(module);
                        break;
                    }
                }
            }

            if (fwdPlusTraj != nullptr)
            {
                no_plus = false;
                for (auto device_fwd : fwdPlusTraj->getTrajectoryDevices())
                {
                    QString fwd_name = device_fwd->getName();
                    if (device_name == fwd_name)
                    {
                        module->setFwdTrajectoryDevice(device_fwd);
                        device_fwd->setBwdConnectorDevice(module);
                        break;
                    }
                }
            }
            else
            {
                no_plus = true;
            }

            if (fwdMinusTraj != nullptr)
            {
                for (auto device_fwd : fwdMinusTraj->getTrajectoryDevices())
                {
                    QString fwd_name = device_fwd->getName();
                    if (device_name == fwd_name)
                    {
                        if (no_plus)
                            module->setFwdTrajectoryDevice(device_fwd);
                        device_fwd->setBwdConnectorDevice(module);
                        break;
                    }
                }
            }

            // TODO конфигурирование?
            /*
            QString cfg_topology_dir = QDir::toNativeSeparators(route_dir) +
                                       QDir::separator() + "topology";
            module->read_config(conn_module, cfg_topology_dir);
            */

            devices.push_back(module);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::step(double t, double dt)
{
    (void) t;
    (void) dt;

    int prev_state_fwd = state_fwd;
    int prev_state_bwd = state_bwd;

    // Если траектория вперёд единственная - делать дальше нечего
    if ((fwdMinusTraj == Q_NULLPTR) || (fwdPlusTraj == Q_NULLPTR))
    {
        state_fwd = 0;
    }
    else
    {
        // Если какая-то траектория спереди занята ПЕ ближе,
        // чем заданная дистанция, ставим стрелку в это направление
        if (fwdPlusTraj->isBusy(0.0, lock_by_busy_distance))
        {
            state_fwd = 2;
        }
        else
        {
            if (fwdMinusTraj->isBusy(0.0, lock_by_busy_distance))
            {
                state_fwd = -2;
            }
            else
            {
                // Переключаем стрелку в требуемое положение
                state_fwd = ref_state_fwd;
            }
        }
    }


    // Если траектория назад единственная - делать дальше нечего
    if ((bwdMinusTraj == Q_NULLPTR) || (bwdPlusTraj == Q_NULLPTR))
    {
        state_bwd = 0;
    }
    else
    {
        // Если какая-то траектория сзади занята ПЕ ближе,
        // чем заданная дистанция, ставим стрелку в это направление
        if (bwdPlusTraj->isBusy(bwdPlusTraj->getLength() - lock_by_busy_distance, bwdPlusTraj->getLength()))
        {
            state_bwd = 2;
        }
        else
        {
            if (bwdMinusTraj->isBusy(bwdMinusTraj->getLength() - lock_by_busy_distance, bwdMinusTraj->getLength()))
            {
                state_bwd = -2;
            }
            else
            {
                // Переключаем стрелку в требуемое положение
                state_bwd = ref_state_bwd;
            }
        }
    }

    if ((prev_state_fwd != state_fwd) || (prev_state_bwd != state_bwd))
    {
        switch_state_t new_state;
        new_state.name = name;
        new_state.state_fwd = state_fwd;
        new_state.state_bwd = state_bwd;
        emit sendSwitchState(new_state.serialize());
    }

    int change_fwd = (sign(prev_state_fwd) != sign(state_fwd)) * sign(state_fwd);
    int change_bwd = (sign(prev_state_bwd) != sign(state_bwd)) * sign(state_bwd);
    for (auto device : devices)
    {
        if (change_fwd > 0)
        {
            bool no_change = true;
            for (auto device_fwd : fwdPlusTraj->getTrajectoryDevices())
            {
                if (device->getName() == device_fwd->getName())
                {
                    device->setFwdTrajectoryDevice(device_fwd);
                    no_change = false;
                    break;
                }
            }
            if (no_change)
                device->setFwdTrajectoryDevice(nullptr);
        }
        if (change_fwd < 0)
        {
            bool no_change = true;
            for (auto device_fwd : fwdMinusTraj->getTrajectoryDevices())
            {
                if (device->getName() == device_fwd->getName())
                {
                    device->setFwdTrajectoryDevice(device_fwd);
                    no_change = false;
                    break;
                }
            }
            if (no_change)
                device->setFwdTrajectoryDevice(nullptr);
        }

        if (change_bwd > 0)
        {
            bool no_change = true;
            for (auto device_bwd : bwdPlusTraj->getTrajectoryDevices())
            {
                if (device->getName() == device_bwd->getName())
                {
                    device->setBwdTrajectoryDevice(device_bwd);
                    no_change = false;
                    break;
                }
            }
            if (no_change)
                device->setBwdTrajectoryDevice(nullptr);
        }
        if (change_bwd < 0)
        {
            bool no_change = true;
            for (auto device_bwd : bwdMinusTraj->getTrajectoryDevices())
            {
                if (device->getName() == device_bwd->getName())
                {
                    device->setBwdTrajectoryDevice(device_bwd);
                    no_change = false;
                    break;
                }
            }
            if (no_change)
                device->setBwdTrajectoryDevice(nullptr);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray Switch::serialize()
{
    QBuffer data;
    data.open(QIODevice::WriteOnly);
    QDataStream stream(&data);

    // Имя коннектора в буфер данных
    stream << name;

    // Сериализуем связанные с этим коннектором траектории
    serialize_connected_trajectory(stream, fwdMinusTraj);
    serialize_connected_trajectory(stream, fwdPlusTraj);
    serialize_connected_trajectory(stream, bwdMinusTraj);
    serialize_connected_trajectory(stream, bwdPlusTraj);

    // Помещаем в бувер состояние стрелки
    stream << state_fwd << state_bwd;

    return data.data();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::deserialize(QByteArray &data, traj_list_t &traj_list)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    // Извлекаем имя коннектора из буфера
    stream >> name;

    // Восстанавливаем связанные с этим коннектором траектории
    fwdMinusTraj = deserialize_connected_trajectory(stream, traj_list);
    fwdPlusTraj = deserialize_connected_trajectory(stream, traj_list);
    bwdMinusTraj = deserialize_connected_trajectory(stream, traj_list);
    bwdPlusTraj = deserialize_connected_trajectory(stream, traj_list);

    // Восстанавливаем статусы стрелки
    stream >> state_fwd;
    stream >> state_bwd;

    fwdTraj = this->getFwdTraj();
    bwdTraj = this->getBwdTraj();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::serialize_connected_trajectory(QDataStream &stream, Trajectory *traj)
{
    // Анализирум наличие на каждом из ответвлений траектории,
    // и если она присутствуем, пишем признак присутствия,
    // а далее помещаем сериализованные данные этой траекторри
    // в буфер
    bool has_traj = traj != Q_NULLPTR;

    if (has_traj)
    {
        stream << has_traj;
        stream << traj->getName();
    }
    else
    {
        // Если ответвление свободно - момещаем просто признак
        stream << has_traj;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory *Switch::deserialize_connected_trajectory(QDataStream &stream,
                                              traj_list_t &traj_list)
{
    // Извлекаем признак наличия траектории
    bool has_traj = false;
    stream >> has_traj;

    Trajectory *traj = Q_NULLPTR;

    //и если она есть
    if (has_traj)
    {
        // И восстанавливаем её данные
        QString traj_name;
        stream >> traj_name;

        // Если в списке есть траектории с таким именем
        if (traj_list.contains(traj_name))
        {
            // Просто возвращаем указатель на нее
            traj = traj_list[traj_name];
            return traj;
        }
        else // в противном случае
        {
            // Содаем траекторию
            traj = new Trajectory;
            traj_list.insert(traj_name, traj);
            return traj;
        }
    }

    return traj;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::setStateFwd(int state)
{
    // Задаём стрелке состояние
    state_fwd = state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::setStateBwd(int state)
{
    // Задаём стрелке состояние
    state_bwd = state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::setRefStateFwd(int state)
{
    // Задаём стрелке требуемое направление
    ref_state_fwd = state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::setRefStateBwd(int state)
{
    // Задаём стрелке требуемое направление
    ref_state_bwd = state;
}
