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
    fwdTraj = traj_list.value(fwd_name, nullptr);

    QString bwd_name;
    cfg.getString(secNode, "bwdTraj", bwd_name);
    bwdTraj = traj_list.value(bwd_name, nullptr);

    if (fwdTraj != nullptr)
    {
        fwdTraj->setBwdConnector(this);
        Journal::instance()->info("Forward trajectory: " + fwdTraj->getName());
    }
    else
    {
        Journal::instance()->info("Joint " + name + " has't incomming trajectory");
    }

    if (bwdTraj != nullptr)
    {
        bwdTraj->setFwdConnector(this);
        Journal::instance()->info("Backward trajectory: " + bwdTraj->getName());
    }
    else
    {
        Journal::instance()->error("Joint " + name + " has't outgoing trajectory");
    }
}

