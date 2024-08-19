#include    <tcp-server.h>
#include    <Journal.h>
#include    <CfgReader.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TcpServer::TcpServer(QObject *parent) : QTcpServer(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TcpServer::~TcpServer()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TcpServer::init(QString cfg_path)
{
    Journal::instance()->info("Strating init TCP-server...");

    CfgReader cfg;

    if (!cfg.load(cfg_path))
    {
        Journal::instance()->error("Can't load server config file from " + cfg_path);
        return false;
    }

    QString secName = "Server";
    int tmp = 0;
    if (cfg.getInt(secName, "port", tmp))
    {
        port = static_cast<quint16>(tmp);
    }

    if (!isListening())
    {
        if (listen(QHostAddress::Any, port))
        {
            Journal::instance()->info(QString("TCP-server listen at port %1").arg(port));
        }
        else
        {
            Journal::instance()->error(QString("Failed start TCP-server at port %1").arg(port));
        }
    }

    return true;
}
