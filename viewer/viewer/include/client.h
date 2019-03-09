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

#ifndef     CLIENT_H
#define     CLIENT_H

#include    "tcp-client.h"
#include    "server-data-struct.h"
#include    "trajectory-element.h"
#include    "settings.h"

#include    <QTimer>
#include    <QSharedMemory>
#include    <osgViewer/Viewer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class NetworkClient : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    NetworkClient(QObject *parent = Q_NULLPTR);

    /// Destructor
    virtual ~NetworkClient();

    /// Client initialization
    void init(const settings_t settings, osgViewer::Viewer *viewer);

private:

    /// Client object
    TcpClient   *tcp_client;

    /// Pointer to OSG viewer
    osgViewer::Viewer *viewer;

    /// Timer for send data requests
    QTimer      timerRequester;

    /// Data structure, reseived from server
    server_data_t   server_data;

    /// Requests time interval (ms)
    int     request_interval;

    /// Buffer for store keyboard state
    QByteArray  key_data;    

private slots:

    void onClientAuthorized();

    void onClientDisconnected();

    void onTimerRequest();

public slots:

    void receiveKeysState(QByteArray data);    
};

#endif // CLIENT_H
