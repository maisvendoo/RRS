//------------------------------------------------------------------------------
//
//      TCP-client for exchange data with simulator physichs
//      (c) maisvendoo, 18/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief TCP-client for chenge data with simulator physichs
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 18/12/2018
 */

#include    "client.h"
#include    "trajectory-element.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
NetworkClient::NetworkClient(QObject *parent) : QObject(parent)
{
    connect(&timerRequester, &QTimer::timeout,
            this, &NetworkClient::onTimerRequest);    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
NetworkClient::~NetworkClient()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NetworkClient::init(const settings_t settings, osgViewer::Viewer *viewer)
{
    request_interval = settings.request_interval;

    this->viewer = viewer;

    timerRequester.setInterval(settings.request_interval);

    tcp_client = new TcpClient();

    connect(tcp_client, &TcpClient::authorized, this, &NetworkClient::onClientAuthorized);
    connect(tcp_client, &TcpClient::disconnectedFromServer, this, &NetworkClient::onClientDisconnected);
   // connect(tcp_client, &TcpClient::logPrint, this, &NetworkClient::logPrint);

    tcp_config_t tcp_config;
    tcp_config.host_addr = QString(settings.host_addr.c_str());
    tcp_config.name = QString(settings.name.c_str());
    tcp_config.port = static_cast<quint16>(settings.port);
    tcp_config.reconnect_interval = settings.reconnect_interval;


    tcp_client->setRecvDataSize(static_cast<qint64>(sizeof (server_data_t)));
    tcp_client->init(tcp_config);
    tcp_client->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NetworkClient::onClientAuthorized()
{
    if (tcp_client->isConnected())
    {
        OSG_INFO << "TCP-CLIENT: Authorization is complete" << std::endl;
        timerRequester.start();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NetworkClient::onClientDisconnected()
{
    OSG_INFO << "TCP-CLIENT: Client is disconnected" << std::endl;
    timerRequester.stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NetworkClient::onTimerRequest()
{
    if (tcp_client->isConnected())
    {
        size_t in_size = static_cast<size_t>(tcp_client->getBufferSize());
        size_t size = sizeof (server_data_t);

        if (in_size == size)
        {
            QByteArray array = tcp_client->getBuffer();
            memcpy(static_cast<void *>(&server_data), array.data(), sizeof (server_data_t));

            if (viewer != nullptr)
            {
                network_data_t *traj_elem = new network_data_t();

                memcpy(&traj_elem->te, &server_data.te, sizeof (traj_element_t) * MAX_NUM_VEHICLES);

                traj_elem->route_id = 1;
                traj_elem->delta_time = server_data.delta_time;

                viewer->getEventQueue()->userEvent(traj_elem);
            }
        }

        tcp_client->sendToServer(ATcp::tcPOSTGET, key_data);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NetworkClient::receiveKeysState(QByteArray data)
{
    key_data = data;
}


