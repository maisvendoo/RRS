#include    "isolated-joint.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
IsolatedJoint::IsolatedJoint(QObject *parent) : Connector(parent)
  , signal(Q_NULLPTR)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
IsolatedJoint::~IsolatedJoint()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void IsolatedJoint::configure(CfgReader &cfg, QDomNode secNode, traj_list_t &traj_list)
{
    Connector::configure(cfg, secNode, traj_list);

    QString fwd_name;
    cfg.getString(secNode, "fwdTraj", fwd_name);
    fwdTraj = traj_list.value(fwd_name, Q_NULLPTR);

    QString bwd_name;
    cfg.getString(secNode, "bwdTraj", bwd_name);
    bwdTraj = traj_list.value(bwd_name, Q_NULLPTR);

    if (fwdTraj != Q_NULLPTR)
    {
        fwdTraj->setBwdConnector(this);
    }

    if (bwdTraj != Q_NULLPTR)
    {
        bwdTraj->setFwdConnector(this);
    }
}

