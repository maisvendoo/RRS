//-----------------------------------------------------------------------------
//
//      Шкала спидометра. Основные линии на шкале
//      (c) РГУПС, ВЖД 12/05/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Шкала спидометра. Основные линии на шкале
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 12/05/2017
 */

#ifndef SSCALEMAJORLINE_H
#define SSCALEMAJORLINE_H

#include <QLabel>
#include <QPainter>

/*!
 * \class SScaleMajorLine
 * \brief Прорисовка основных линий скоростей на шкале спидометра
 */
class SScaleMajorLine : public QLabel
{
public:

    /*!
     * \brief Конструктор
     * \param len - длина линии
     * \param width - ширина линии
     * \param color - цвет линии
     * \param col - количество линий
     * \param angleStep - угол шага линий
     * \param angleZeroPoint - начальный угол
     * \param arcLimitWidth - ширина дуги ограничения
     */
    SScaleMajorLine( int len, int width, QColor color, int col,
                     double angleStep, double angleZeroPoint,
                     int arcLimitWidth,
                     QWidget* parent = Q_NULLPTR );
    /// Деструктор
    ~SScaleMajorLine();

    /// Нарисовать линии
    void drawMajorLine();

private:

    QImage img_; ///< картинка для линий

    double w_2_; ///< половина ширины класса
    double h_2_; ///< половина высоты класса


    int lineLength_;    ///< длина линии
    int lineWidth_;     ///< ширина линии
    QColor lineColor_;  ///< цвет линии
    int lineCount_;     ///< количество линий
    double angleStep_;  ///< шаг угла
    double aZeroPoint_; ///< Угол стартовой точки (0 - начало шкалы спидометра)
    int arcLimitWidth_; ///< ширина дуги ограничения


};

#endif // SSCALEMAJORLINE_H
