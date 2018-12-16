//------------------------------------------------------------------------------
//
//      Network communication with simulator's server
//      (c) maisvendoo, 10/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief
 * \copyright
 * \author
 * \date
 */

#include    "network.h"

#include    <iostream>
#include    <QString>

#include    "trajectory-element.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
NetworkClient::NetworkClient(QObject *parent) : QObject(parent)
{
    connect(&timerRequester, &QTimer::timeout,
            this, &NetworkClient::onTimerRequester);        

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
void NetworkClient::init(const settings_t &settings, osgViewer::Viewer *viewer)
{
    request_interval = settings.request_interval;

    this->viewer = viewer;

    // Initialize requester timer
    timerRequester.setInterval(settings.request_interval);

    tcp_cilent = new TcpClient();

    connect(tcp_cilent, &TcpClient::authorized, this, &NetworkClient::onClientAuthorized);
    connect(tcp_cilent, &TcpClient::disconnectedFromServer, this, &NetworkClient::onClientDissconnected);

    tcp_config_t tcp_config;
    tcp_config.host_addr = QString(settings.host_addr.c_str());
    tcp_config.name = QString(settings.name.c_str());
    tcp_config.port = static_cast<quint16>(settings.port);
    tcp_config.reconnect_interval = settings.reconnect_interval;

    tcp_cilent->init(tcp_config);
    tcp_cilent->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NetworkClient::onClientAuthorized()
{
    if (tcp_cilent->isConnected())
    {
        timerRequester.start();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NetworkClient::onClientDissconnected()
{
    traj_element_t *traj_elem = new traj_element_t();
    traj_elem->route_id = 0;
    viewer->getEventQueue()->userEvent(traj_elem);

    timerRequester.stop();    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void NetworkClient::onTimerRequester()
{
    if (tcp_cilent->isConnected())
    {
        if (tcp_cilent->getBufferSize() == sizeof (server_data_t))
        {
            // Get received data
            QByteArray array = tcp_cilent->getBuffer();
            // Copy data to target structure
            memcpy(&server_data, array.data(), sizeof (server_data_t));

            if (viewer != nullptr)
            {
                // Send data to camera trajectory handler
                traj_element_t *traj_elem = new traj_element_t();

                traj_elem->route_id = server_data.route_id;
                traj_elem->coord_end = server_data.railway_coord;
                traj_elem->delta_time = static_cast<float>(request_interval) / 1000.0f;

                viewer->getEventQueue()->userEvent(traj_elem);
            }
        }

        // Send get data request
        tcp_cilent->sendToServer(ATcp::tcGET);
    }
}
