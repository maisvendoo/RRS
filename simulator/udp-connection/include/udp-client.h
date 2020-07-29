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

    void init(QString& cfg_path);

    bool isConnected();

private:
    QUdpSocket *clientSocket;

    udp_server_data_t client_data;

    int port;

    void load_config(QString &path);

private slots:
    void receive();
};

#endif // UDP_CLIENT_H
