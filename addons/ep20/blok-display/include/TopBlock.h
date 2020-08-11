//-----------------------------------------------------------------------------
//
//      Верхний блок БЛОКа
//      (c) РГУПС, ВЖД 30/03/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Класс "Верхний блок" БЛОКа
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 30/03/2017
 */

#ifndef TOPBLOCK_H
#define TOPBLOCK_H

#include <QMap>
#include <QTimer>

#include "Speedometer.h"
#include "InformPart.h"
#include "InformPart2.h"
#include "InformPartPressure.h"
#include "InformPartTriangle.h"
#include "structures-BLOK.h"

//-----------------------------------------------------------------------------
//  Структура элементов информационного блока
//-----------------------------------------------------------------------------
struct ip_t
{
    QString coordinate;
    QString station;
    QString time;
//    QString grafic;
    QString P;
    QString P2;
    QString chanel;
    QString numRoad;
    QString acceleration;
    QString distanceTargetSAUT;
    QString coeffBraking;
    QString distanceTarget;
    QString typeTarget;
    QString nameTarget;
    QString zz;

    ip_t()
    {
        coordinate          = "coordinate";
        station             = "station";
        time                = "time";
//        grafic          = "grafic";
        P                   = "P";
        P2                  = "P2";
        chanel              = "chanel";
        numRoad             = "numRoad";
        acceleration        = "acceleration";
        distanceTargetSAUT  = "distanceSAUT";
        coeffBraking        = "coeffBraking";
        distanceTarget      = "distance";
        typeTarget          = "type";
        nameTarget          = "name";
        zz                  = "zz";
    }
};

//-----------------------------------------------------------------------------
//  Структура элементов информационного блока 2
//-----------------------------------------------------------------------------
struct ip2_t
{
    QString BC;
    QString TM;
    QString UR;

    ip2_t()
    {
        BC = "BC";
        TM = "TM";
        UR = "UR";
    }
};


/*!
 * \class TopBlock
 * \brief Класс, описывающий верхний блок БЛОКа
 */
class TopBlock : public QWidget
{
    Q_OBJECT

public:

    /*!
     * \brief Конструктор
     * \param geo - размер и положение блока
     */
    TopBlock(QRect geo, QWidget* parent = Q_NULLPTR, QString config_dir = "");
    /// Деструктор
    ~TopBlock();


    /// Установить значения элементов информационного блока
    void set_ipVal(ip_val_t* val);

    /// Установить значения элементов информационного блока 2
    void set_ip2Val(ip2_val_t* val);

    /// Зажечь "проверка бдительности" (жёлтый треугольник)
    void setTriangleYellow(bool isSignal);

    /// зажечь ТСКБМ (красный треугольник)
    void setTriangleRed(bool isSignal);

    /// Установить текущее и следующее ограничение скорости
    void setSpeedLimits(int curSpeedLimit, int nextSpeedLimit);

    /// Установить текущую скорость
    void setCurSpeed(double speed);


//private slots:
//    void onTimer(); ///< Установить текущее время на блоке

private:
    /// Элемент блока информации
    InformPart* informPart_;
    /// элемент блока информации (давление)
    InformPartPressure* ipPressureUR_;
    InformPartPressure* ipPressureTM_;
    InformPartPressure* ipPressureTC_;
    /// Элемент блока информации (треугольник)
    InformPartTriangle* ipTriangleRed_;
    InformPartTriangle* ipTriangleYellow_;
    InformPartTriangle* ipTriangle_;
    /// Спидометр
    Speedometer* speedometer_;

    /// Ассоциативный массив элементов информационных блоков
    QMap<QString, InformPart*> informPartMap_;

//    QLabel* labelCurTime_;  ///< виджет для отображения времени на блоке
//    void drawCurentTime_(); ///< нарисовать элемент отображения времени блока

    ip_t* ip_;      ///< структура элементов информационного блока
    ip2_t* ip2_;    ///< структура элементов информационного блока 2


    QString getCoordinateStr_(double coordinate);
    QString getAccelerationStr_(double a);


    // --- cfg параметры ТРЕУГОЛЬНИКА --- //
    /// Параметры ЖЕЛТОГО ТРЕУГОЛЬНИКА
    double  widthTriangle_Yellow_;  ///< ширина основания
    double  lengthTriangle_Yellow_; ///< длина треугольника
    QString colorTriangle_Yellow_;  ///< цвет треугольника

    /// Параметры КРАСНОГО ТРЕУГОЛЬНИКА
    double  widthTriangle_Red_;  ///< ширина основания
    double  lengthTriangle_Red_; ///< длина треугольника
    QString colorTriangle_Red_;  ///< цвет треугольника
    // --------------------- //

    /// Чтение конфигураций
    bool loadIPTriangleCfg(QString cfg_path);

};

#endif // TOPBLOCK_H
