//-----------------------------------------------------------------------------
//
//      Общая структура для приёма данных от сервера
//      (c) РГУПС, ВЖД 06/06/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Общая структура для приёма данных от сервера
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 06/06/2017
 */

#ifndef STRUCTURESBLOK_H
#define STRUCTURESBLOK_H

#include <QString>
#include <QMetaType>

//-----------------------------------------------------------------------------
//  Структура значений элементов информационного блока
//-----------------------------------------------------------------------------
#pragma pack(push, 1)
struct ip_val_t
{
    double coordinate;      ///< координата
    char time[9];
    char grafic[9];
//    QString P;
//    QString P2;
    char station[51];       ///< станция
    char numRoad[15];       ///< номер пути
    char chanel[7];         ///< канал
    double acceleration;    ///< ускорение
    int distanceTargetSAUT; ///< расстояние до цели САУТ
    double coeffBraking;    ///< коэф. торм.
    int distanceTarget;     ///< расстояние до цели
    char typeTarget[36];    ///< вид цели
    char nameTarget[36];    ///< название цели
    char zz[256];           ///<

    ip_val_t()
    {
        coordinate      = 0.0;
        strcpy(time, "00:00:00");
        strcpy(grafic, "00:00:00");
//        P;
//        P2;
        strcpy(station, "Станция1");
        strcpy(numRoad, "путь1");
        strcpy(chanel, "25Гц");
        acceleration = 0.01;
        distanceTargetSAUT = 0;
        coeffBraking = 0.0;
        distanceTarget = 0;
        strcpy(typeTarget, "Вид цели");
        strcpy(nameTarget, "Название цели");
        strcpy(zz, "");
    }
};
#pragma pack(pop)

//-----------------------------------------------------------------------------
//  Структура значений элементов информационного блока 2
//-----------------------------------------------------------------------------
#pragma pack(push, 1)
struct ip2_val_t
{
    double BC;  ///< давление тормозного цилиндра
    double TM;  /// давление тормозной магистрали
    double UR;  /// давление уравнительного резервуара

    ip2_val_t()
    {
        BC = 0.5;
        TM = 0.0;
        UR = 0.0;
    }
};
#pragma pack(pop)

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#pragma pack(push, 1)
struct other_val_t
{

    bool signalControlLookOut;  ///< сигнал "проверка бдительности"
    bool signalTSKBM;           ///< сигнал тевоги ТСКБМ
    int curSpeedLimit;          ///< текущее ограничение скорости
    int nextSpeedLimit;         ///< следующее ограничение скорости
    double curSpeed;            ///< текущая скорость поезда
    short semaphor_color;       ///< сигнал семафора
    short semaphor_free_bloks;  ///<

    other_val_t()
    {
        signalTSKBM = false;
        signalControlLookOut = false;
        curSpeedLimit = 0;
        nextSpeedLimit = 0;
        curSpeed = 0;
        semaphor_color = 0;
        semaphor_free_bloks = 1;
    }
};
#pragma pack(pop)


//-----------------------------------------------------------------------------
//  Структура для приёма данных с сервера
//-----------------------------------------------------------------------------
#pragma pack(push, 1)
struct structs_BLOK_t
{
    bool            wasSendData; ///< были ли отправленны данные с сервера
    ip_val_t        ip_val;
    ip2_val_t       ip2_val;
    other_val_t     other_val;

    structs_BLOK_t()
    {
        wasSendData = false;
    }

};
#pragma pack(pop)




Q_DECLARE_METATYPE(structs_BLOK_t)


#endif // STRUCTURESBLOK_H
