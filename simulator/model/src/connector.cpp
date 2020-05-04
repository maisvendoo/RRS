#include    "connector.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Connector::Connector(QObject *parent) : QObject(parent)
  , fwdTraj(Q_NULLPTR)
  , bwdTraj(Q_NULLPTR)
  , state(0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Connector::~Connector()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Connector::configure(CfgReader &cfg, QDomNode secNode, traj_list_t &traj_list)
{
    Q_UNUSED(traj_list)

    cfg.getString(secNode, "Name", name);
    cfg.getInt(secNode, "state", state);
}
