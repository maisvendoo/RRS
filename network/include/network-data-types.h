#ifndef NETWORK_DATA_TYPES_H
#define NETWORK_DATA_TYPES_H

#include <QByteArray>
#include <QtTypes>

class QTcpSocket;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum StructureType : uint8_t
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

    STYPE_COMMAND_SWITCH_CONTROL,
    STYPE_COMMAND_SIGNAL_CONTROL,
    STYPE_COMMAND_BUILD_ROUTE,
    STYPE_COMMAND_TRAIN_ROUTE,
    STYPE_COMMAND_SHUNTING_ROUTE,
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
    STYPE_COMMAND_SET_SIMULATION_SPEED,
    STYPE_SEND_VEHICLE_CONTROL_COMMAND,

    // Новые типы добавляются ТОЛЬКО В КОНЕЦ (обратная совместимость)
    STYPE_REQUEST_DIAGNOSTICS_UPDATE,   ///< Запрос снимка диагностики (F3/F4)
    STYPE_DIAGNOSTICS_UPDATE,           ///< Снимок диагностики составов

    // ТЗ "RP-сервер" (п.5, 6, 9): табельный номер, назначение игрока
    // на поезд, загрузка сейва сессии организатором
    STYPE_SEND_TAB_NUMBER,              ///< Клиент сообщает табельный номер
    STYPE_COMMAND_SET_TRAIN_TAB,        ///< Организатор: назначить табельный поезду
    STYPE_COMMAND_LOAD_SESSION          ///< Организатор: загрузить сейв сессии
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct network_data_t
{
    network_data_t();

    /// Размер данных
    uint32_t data_size;

    /// Тип передаваемой/принимаемой структуры
    StructureType stype;

    /// Сжатие
    bool is_compression;

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
    double diagnostics_update_interval;      ///< Интервал снимков диагностики, с
    double diagnostics_update_prev_time;     ///< Время последнего снимка, с
    int tab_number = -1;                     ///< Табельный номер игрока (ТЗ "RP-сервер", п.9; -1 - неизвестен)
    QTcpSocket* socket;
    network_data_t received_data;
};

#endif
