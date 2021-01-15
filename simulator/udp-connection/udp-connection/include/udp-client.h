//-----------------------------------------------------------------------------
//
//
//
//
//
//-----------------------------------------------------------------------------
/*!
 *  \file
 *  \brief
 *  \copyright
 *  \author
 *  \date
 */

#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include <QtGlobal>
#include <QObject>
#include <QDataStream>
#include <QUdpSocket>
#include <QNetworkDatagram>

#include "CfgReader.h"
#include "udp-data-struct.h"

#if defined(UDP_CLIENT_LIB)
# define UDP_CLIENT_EXPORT Q_DECL_EXPORT
#else
# define UDP_CLIENT_EXPORT Q_DECL_IMPORT
#endif

/*!
 *  \class UdpClient
 *  \brief
 */
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class UDP_CLIENT_EXPORT UdpClient Q_DECL_FINAL : public QObject
{
    Q_OBJECT

public:
    explicit UdpClient();

    ~UdpClient();

    void init(const QString& cfg_path);

    bool isConnected();

    void sendData(const QByteArray &data);

    /// Отключение прокси
    void setNoProxy(bool no_proxy = true);

    void setPort(unsigned short port) { client_port = port; }

    udp_server_data_t getClientData() { return client_data; }

private:
    QUdpSocket *clientSocket;

    QByteArray incomingData_;

    udp_server_data_t client_data;

//    QHostAddress client_host;
    QHostAddress server_host;

    unsigned short client_port;
    unsigned short server_port;

    void load_config(const QString& path);

signals:
    void authorized();

    void disconnectedFromServer();

public slots:
    void readPendingDatagrams();
};

#endif // UDP_CLIENT_H
