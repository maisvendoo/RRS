#include    "switch.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Switch::Switch(QObject *parent) : Connector(parent)
  , fwdMinusTraj(Q_NULLPTR)
  , fwdPlusTraj(Q_NULLPTR)
  , bwdMinusTraj(Q_NULLPTR)
  , bwdPlusTraj(Q_NULLPTR)
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
        return fwdPlusTraj;

    if (state == -1)
        return fwdMinusTraj;

    return Q_NULLPTR;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory *Switch::getBwdTraj() const
{
    if (state == 1)
        return bwdPlusTraj;

    if (state == -1)
        return bwdMinusTraj;

    return Q_NULLPTR;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::configure(CfgReader &cfg, QDomNode secNode, traj_list_t &traj_list)
{
    Connector::configure(cfg, secNode, traj_list);

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

    if (bwdMinusTraj != Q_NULLPTR)
    {
        bwdMinusTraj->setFwdConnector(this);
    }

    if (bwdPlusTraj != Q_NULLPTR)
    {
        bwdPlusTraj->setFwdConnector(this);
    }

    if (fwdMinusTraj != Q_NULLPTR)
    {
        fwdMinusTraj->setBwdConnector(this);
    }

    if (fwdPlusTraj != Q_NULLPTR)
    {
        fwdPlusTraj->setBwdConnector(this);
    }
}
