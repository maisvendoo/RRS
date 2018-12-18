#ifndef     CLIENT_H
#define     CLIENT_H

#include    "tcp-client.h"
#include    "server-data-struct.h"
#include    "settings.h"

#include    <QTimer>
#include    <osgViewer/Viewer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class NetworkClient : public QObject
{
    Q_OBJECT

public:

    NetworkClient(QObject *parent = Q_NULLPTR);

    virtual ~NetworkClient();

    void init(const settings_t settings, osgViewer::Viewer *viewer);

private:

    TcpClient   *tcp_client;

    osgViewer::Viewer *viewer;

    QTimer      timerRequester;

    server_data_t   server_data;

    int     request_interval;

private slots:

    void onClientAuthorized();

    void onClientDisconnected();

    void onTimerRequest();
};

#endif // CLIENT_H
