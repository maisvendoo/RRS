#include    <switch.h>
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
    if (state_fwd == 1)
    {
        if (fwdPlusTraj != Q_NULLPTR)
            return fwdPlusTraj;
        else
        {
            if (fwdMinusTraj != Q_NULLPTR)
                return fwdMinusTraj;
        }
    }

    if (state_fwd == -1)
    {
        if (fwdMinusTraj != Q_NULLPTR)
            return fwdMinusTraj;
        {
            if (fwdPlusTraj != Q_NULLPTR)
                return fwdPlusTraj;
        }
    }

    return Q_NULLPTR;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory *Switch::getBwdTraj() const
{
    if (state_bwd == 1)
    {
        if (bwdPlusTraj != Q_NULLPTR)
            return bwdPlusTraj;
        else
        {
            if (bwdMinusTraj != Q_NULLPTR)
                return bwdMinusTraj;
        }
    }

    if (state_bwd == -1)
    {
        if (bwdMinusTraj != Q_NULLPTR)
            return bwdMinusTraj;
        {
            if (bwdPlusTraj != Q_NULLPTR)
                return bwdPlusTraj;
        }
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
    }

    if (outputs_count == 0)
    {
        Journal::instance()->error("Switch " + name + " has't outgoing trajectories!!!");
    }
    else
    {
        Journal::instance()->info("Outgoing trajectories: " + QString("%1").arg(outputs_count));
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
