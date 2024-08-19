#include    <connector.h>
#include    <Journal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Connector::Connector(QObject *parent) : QObject(parent)
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
void Connector::configure(CfgReader &cfg,
                          QDomNode secNode,
                          traj_list_t &traj_list)
{
    Q_UNUSED(traj_list)

    cfg.getString(secNode, "Name", name);
    cfg.getInt(secNode, "state", state);

    Journal::instance()->info("Connector " + name + " will be initialized...");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray Connector::serialize()
{
    QBuffer data;
    data.open(QIODevice::WriteOnly);
    QDataStream stream(&data);

    stream << name.length() << name;

    return data.data();
}
