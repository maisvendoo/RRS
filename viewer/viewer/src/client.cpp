//------------------------------------------------------------------------------
//
//      TCP-client for chenge data with simulator physichs
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

    tcp_config_t tcp_config;
    tcp_config.host_addr = QString(settings.host_addr.c_str());
    tcp_config.name = QString(settings.name.c_str());
    tcp_config.port = static_cast<quint16>(settings.port);
    tcp_config.reconnect_interval = settings.reconnect_interval;

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
        timerRequester.start();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NetworkClient::onClientDisconnected()
{
    timerRequester.stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NetworkClient::onTimerRequest()
{
    if (tcp_client->isConnected())
    {
        if (tcp_client->getBufferSize() == sizeof (server_data_t))
        {
            QByteArray array = tcp_client->getBuffer();
            memcpy(&server_data, array.data(), sizeof (server_data_t));

            if (viewer != nullptr)
            {
                traj_element_t *traj_elem = new traj_element_t();

                traj_elem->route_id = 1;
                traj_elem->coord_end = server_data.cabine_coord;
                traj_elem->delta_time = static_cast<float>(request_interval) / 1000.0f;

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
