#ifndef     TCP_SERVER_H
#define     TCP_SERVER_H

#include    <QTcpServer>
#include    <network-export.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class NETWORK_EXPORT TcpServer : public QTcpServer
{
    Q_OBJECT

public:

    TcpServer(QObject *parent = Q_NULLPTR);

    ~TcpServer();

    bool init(QString cfg_path);

private:

    quint16 port = 1992;
};

#endif
