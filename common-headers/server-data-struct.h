//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief
 * \copyright
 * \author
 * \date
 */

#ifndef     SERVER_DATA_STRUCT_H
#define     SERVER_DATA_STRUCT_H

#include    <qglobal.h>

/*!
 * \struct
 * \brief Data structure, received from server
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#pragma pack(push, 1)

struct server_data_t
{
    quint64             count;           ///< Счетчик сообщений 8
    float               railway_coord;   ///< Текущая координата (м) 12
    float               velocity;        ///< Скорость (м/с) 16
    float               time;            ///< Модельное время 20
    float               deltaTime;       ///< Интервал между отправками 24
    int                 dir;             ///< Направление изменения координаты 28
    quint32             route_id;        ///< Идентификатор маршрута 32
    unsigned char       control_flags1;  ///< Прожектор тускло 33
    unsigned char       control_flags2; ///< Прожектор ярко 34

    server_data_t()
        : count(0)
        , railway_coord(0.0)
        , velocity(0.0)
        , time(0.0)
        , deltaTime(0.0)
        , dir(0)
        , route_id(0)
        , control_flags1(0)
        , control_flags2(0)
    {

    }
};

#pragma pack(pop)

#endif // SERVER_DATA_STRUCT_H
