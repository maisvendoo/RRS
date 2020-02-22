#include    "sim-client.h"
#include    "tcp-client.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SimTcpClient::SimTcpClient(QObject *parent) : QObject(parent)
  , tcp_client(new TcpClient)
{
    qRegisterMetaType<sim_train_data_t>();
    qRegisterMetaType<sim_dispatcher_data_t>();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SimTcpClient::~SimTcpClient()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SimTcpClient::isConnected() const
{
    return tcp_client->isConnected();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SimTcpClient::getRecvData(sim_dispatcher_data_t &disp_data)
{
    if (tcp_client == Q_NULLPTR)
        return;

    QByteArray recv_data = tcp_client->getBuffer();

    if (recv_data.size() != sizeof (sim_dispatcher_data_t))
    {
        return;
    }

    sim_dispatcher_data_t *dsp = static_cast<sim_dispatcher_data_t *>(static_cast<void *>(recv_data.data()));

    disp_data = *dsp;

    return;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SimTcpClient::sendTrainData(sim_train_data_t train_data)
{
    if (tcp_client == Q_NULLPTR)
        return;        

    tcp_client->sendToServer(ATcp::tcPOSTGET, ATcp::toByteArrayHard(train_data));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SimTcpClient::init(const tcp_config_t &tcp_config)
{
    tcp_client->init(tcp_config);
    tcp_client->setNoProxy();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SimTcpClient::start()
{
    if (tcp_client != Q_NULLPTR)
        tcp_client->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SimTcpClient::stop()
{
    if (tcp_client != Q_NULLPTR)
    {
        tcp_client->stop();
    }
}
