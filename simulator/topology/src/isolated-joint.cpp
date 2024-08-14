#include    <isolated-joint.h>
#include    <Journal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
IsolatedJoint::IsolatedJoint(QObject *parent) : Connector(parent)  
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

    Journal::instance()->info("Connector type: joint");

    QString fwd_name;
    cfg.getString(secNode, "fwdTraj", fwd_name);
    fwdTraj = traj_list.value(fwd_name, Q_NULLPTR);

    QString bwd_name;
    cfg.getString(secNode, "bwdTraj", bwd_name);
    bwdTraj = traj_list.value(bwd_name, Q_NULLPTR);

    if (fwdTraj != Q_NULLPTR)
    {
        fwdTraj->setBwdConnector(this);
        Journal::instance()->info("Forward trajectory: " + fwdTraj->getName());
    }
    else
    {
        Journal::instance()->info("Joint " + name + " has't incomming trajectory");
    }

    if (bwdTraj != Q_NULLPTR)
    {
        bwdTraj->setFwdConnector(this);
        Journal::instance()->info("Backward trajectory: " + bwdTraj->getName());
    }
    else
    {
        Journal::instance()->error("Joint " + name + " has't outgoing trajectory");
    }
}

