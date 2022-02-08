#ifndef     SIM_CLIENT_H
#define     SIM_CLIENT_H

#include    <QObject>
#include    <QMetaType>

#include    "tcp-client-structs.h"
#include    "shared-sim-dispatcher-stuctures.h"

Q_DECLARE_METATYPE(sim_dispatcher_data_t)

Q_DECLARE_METATYPE(sim_train_data_t)

class TcpClient;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SimTcpClient : public QObject
{
    Q_OBJECT

public:

    SimTcpClient(QObject *parent = Q_NULLPTR);

    ~SimTcpClient();

    bool isConnected() const;

    void init(const tcp_config_t &tcp_config);

public slots:

    void start();

    void stop();

    void getRecvData(sim_dispatcher_data_t &disp_data);

    void sendTrainData(sim_train_data_t train_data);

private:

    TcpClient               *tcp_client;

    sim_dispatcher_data_t    disp_data;
};

#endif
