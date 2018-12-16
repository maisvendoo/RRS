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

#include    "tcp-client.h"
#include    "server-data-struct.h"
#include    "settings.h"

#include    <QTimer>
#include    <osgViewer/Viewer>

#ifndef     NETWORK_H
#define     NETWORK_H

/*!
 * \class
 * \brief TCP-celint
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class NetworkClient : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    explicit NetworkClient(QObject *parent = Q_NULLPTR);

    /// Destructor
    virtual ~NetworkClient();

    /// Client initialization
    void init(const settings_t &settings, osgViewer::Viewer *viewer);

private:

    /// TCP-client object
    TcpClient   *tcp_cilent;

    /// Pointer to viewer object
    osgViewer::Viewer *viewer;

    /// Timer for connection control
    QTimer      timerRequester;

    /// Data received from server
    server_data_t   server_data;

    int request_interval;

private slots:

    /// Authorization handler
    void onClientAuthorized();

    /// Disconnection handler
    void onClientDissconnected();

    /// Timer handler
    void onTimerRequester();
};

#endif // NETWORK_H
