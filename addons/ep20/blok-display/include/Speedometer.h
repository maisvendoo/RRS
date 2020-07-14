//-----------------------------------------------------------------------------
//
//      Спидометр
//      (c) РГУПС, ВЖД 31/03/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Класс элемента "Спидометр"
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 31/03/2017
 */

#ifndef SPEEDOMETER_H
#define SPEEDOMETER_H


#include <QLabel>
#include <QtMath>
#include <QTimer>

#include "SArrow.h"
//#include "STriangle.h"
#include "SScale.h"
#include "SScaleMajorLine.h"
#include "SArcLimit.h"


/*!
 * \class Speedometer
 * \brief Класс, описывающий спидометр
 */
class Speedometer : public QLabel
{
    Q_OBJECT

public:

    /*!
     * \brief Конструктор
     * \param geo - геометрия элемента: размер, положение
     */
    Speedometer(QRect geo, QWidget *parent = Q_NULLPTR, QString config_dir = "");
    /// Деструктор
    ~Speedometer();

    /*!
     * \brief Установить текущую скорость
     * \param speed - скорость
     */
    void setSpeedCur(double speed);

//    /// Установить ограничение скорости
//    void setSpeedLimit(int speedLimit);

    /*!
     * \brief Установить ограничения скорости
     * \param speedLimit - значение текущего ограничения
     * \param nextSpeedLimit - значение следующего ограничения
     */
    void setSpeedLimits(int speedLimit, int nextSpeedLimit);

//    /// Установить заданную скорость с рукоятки
//    void setSpeed_Yellow(int speedLimit_Yellow);

//    /// Установить zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz
//    void setSpeed_Red(int speedLimit_Red);

public slots:
    /// Моргание значения тек. скорости, при приближении к значению ограничения
    void onTimer();

private:
    QTimer* timerForBlink_; ///< таймер, для моргания значения тек. скорости
    int     timeCount_;     ///< счетчик для таймера

    SArrow* sArrow_;                    ///< СТРЕЛКА
//    STriangle* sTriangleYellow_;        // ЖЕЛТЫЙ ТРЕУГОЛЬНИК
//    STriangle* sTriangleRed_;           // КРАСНЫЙ ТРЕУГОЛЬНИК
    SScale* sScale_;                    ///< ШКАЛА
    SScaleMajorLine* sScaleMajorLine_;  ///< ОСНОВНЫЕ ЛИНИИ НА ШКАЛЕ
    SArcLimit* sArcLimit_;              ///< ДУГА ОГРАНИЧЕНИЯ

    /// виджет для отображения значения тек. скорости в центре спидометра
    QLabel* labelValVCenter_;
    /// виджет для отображения знач. ограничения скорости в центре спидометра
    QLabel* labelSpeedLimit_;


    int     speedCurOld_i_;     ///< предыдущая текущая скорость
    double  speedCurOld_d_;     ///< предыдущая текущая скорость
    int     speedLimitOld_;     ///< предыдущее ограничение скорости
//    int     speedRefOld_;       ///< предыдущая заданная скорость с рукоятки
//    int     zzzOld_;            ///< предыдущая zzzzzzzzzz
//    int speedLimit_; ///< текущее значение ограничения скорости


    double aBeginPoint_; ///< угол начальной точки (0 на шкале спидометра)
    double aStep_;       ///< шаг угла


    // --- cfg параметры --- //
    /// основные черточки на шкале спидометра
    QString majorLine_color_;   ///< цвет
    int     majorLine_width_;   ///< ширина
    int     majorLine_length_;  ///< длина



    int valV_max_;       ///< макс. скорость
    int valV_step_;      ///< шаг скорости
    int arc_splitAngle_; ///< длина дуги-разрыва
    int arc_splitPoint_; ///< угол центра разрыва дуги


    /// текущая скорость на центральном круге
    int     labelValVCenter_x_;         ///< положение по оси OX
    int     labelValVCenter_y_;         ///< положение по оси OY
    QString labelValVCenter_color_;     ///< цвет
    int     labelValVCenter_fontSize_;  ///< размер шрифта

    /// ограничение скорости под центральным кругом спидометра
    int     labelSpeedLimit_x_;         ///< положение по оси OX
    int     labelSpeedLimit_y_;         ///< положение по оси OY
    QString labelSpeedLimit_color_;     ///< цвет
    int     labelSpeedLimit_fontSize_;  ///< размер шрифта

    // --------------------- //

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


    /*!
     * \brief Отрисовка скорости на центральном круге спидометра
     * \param label - видждет для отображения
     * \param x - координата оси OX
     * \param y - координата оси OY
     * \param color - цвет
     * \param fontSize - размер текста
     */
    void myDrawValVCenter_( QLabel* label, int x, int y, QString color,
                            int fontSize );


    /*!
     * \brief Установка границ скоростей
     * \param valSpeed - значение текущей скорости
     */
    inline double setSpeedRange_(double valSpeed);

    /*!
     * \brief Парсинг значения скорости в 3х-значное число
     * \param valSpeed - значение текущей скорости
     * \return - текстовое 3х-значное значение текущей скорости
     */
    inline QString parsSpeedCur_(double valSpeed);


    /// Чтение конфигураций
    bool loadSpeedometerCfg(QString cfg_path);
    bool loadSTriangleCfg(QString cfg_path);

};

#endif // SPEEDOMETER_H
