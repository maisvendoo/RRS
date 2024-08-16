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
    if (state == 1)
    {
        if (fwdPlusTraj != Q_NULLPTR)
            return fwdPlusTraj;
        else
        {
            if (fwdMinusTraj != Q_NULLPTR)
                return fwdMinusTraj;
        }
    }

    if (state == -1)
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
    if (state == 1)
    {
        if (bwdPlusTraj != Q_NULLPTR)
            return bwdPlusTraj;
        else
        {
            if (bwdMinusTraj != Q_NULLPTR)
                return bwdMinusTraj;
        }
    }

    if (state == -1)
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
