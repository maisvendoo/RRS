#ifndef NETWORK_DATA_TYPES_H
#define NETWORK_DATA_TYPES_H

#include <QByteArray>
#include <QtTypes>

class QTcpSocket;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum StructureType
{
    STYPE_EMPTY_DATA,

    STYPE_REQUEST_PLAYERS_INFO,

    STYPE_REQUEST_ROUTE_INFO,
    STYPE_REQUEST_TOPOLOGY_DATA,
//    STYPE_REQUEST_TOPOLOGY_UPDATE,

    STYPE_REQUEST_SIGNALS_DATA,
//    STYPE_REQUEST_SIGNALS_UPDATE,

    STYPE_REQUEST_VEHICLES_INFO,
    STYPE_REQUEST_TRAINS_UPDATE,
    STYPE_REQUEST_VEHICLES_POS_UPDATE,
    STYPE_REQUEST_VEHICLES_STATE_UPDATE,
    STYPE_REQUEST_VEHICLE_CONTROLLED_UPDATE,

    STYPE_COMMAND_SWITCH_STATE,
    STYPE_COMMAND_OPEN_SIGNAL,
    STYPE_COMMAND_CLOSE_SIGNAL,
    STYPE_COMMAND_VEHICLE_CONTROL,

//    STYPE_PLAYERS_INFO,
    STYPE_PLAYERS_UPDATE,

    STYPE_ROUTE_INFO,
    STYPE_TOPOLOGY_DATA,
//    STYPE_TOPOLOGY_STATE,
    STYPE_SWITCH_UPDATE,
    STYPE_TRAJ_BUSY_UPDATE,

    STYPE_SIGNALS_DATA,
//    STYPE_SIGNALS_STATE,
    STYPE_SIGNAL_UPDATE,

    STYPE_VEHICLES_INFO,
    STYPE_TRAINS_UPDATE,
    STYPE_VEHICLES_POS_UPDATE,
    STYPE_VEHICLES_STATE_UPDATE,
    STYPE_VEHICLE_CONTROLLED_UPDATE,

    STYPE_COMMAND_RENAME_TRAIN,
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct network_data_t
{
    network_data_t();

    /// Тип передаваемой/принимаемой структуры
    StructureType stype;

    /// Размер данных
    qsizetype data_size;

    /// Сериализованные данные
    QByteArray data;

    /// Сериализуем, подготоваливая кадр, передаваемый по сети
    QByteArray serialize();

    void deserialize(QByteArray& data);
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct client_data_t
{
    client_data_t();

    int id;
    double pos_update_interval;
    double pos_update_prev_time;
    double state_update_interval;
    double state_update_prev_time;
    double controlled_update_interval;
    double controlled_update_prev_time;
    double players_update_interval;
    double players_update_prev_time;
    QTcpSocket* socket;
    network_data_t received_data;
};

#endif
