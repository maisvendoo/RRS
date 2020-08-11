//-----------------------------------------------------------------------------
//
//      Спидометр. Треугольник ограничения
//      (c) РГУПС, ВЖД 02/05/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Класс "Треугольник ограничения" спидометра
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 02/05/2017
 */

#ifndef STRIANGLE_H
#define STRIANGLE_H


#include <QLabel>
#include <QPainter>

/*!
 * \class STriangle
 * \brief Класс, описывающий треугольник ограничения спидометра
 */
class STriangle : public QLabel
{

public:
    /// Конструктор
    STriangle(QWidget *parent = Q_NULLPTR);
    /// Деструктор
    ~STriangle();

    /*!
     * \brief Вращение треугольника
     * \param angle - угол поворота
     */
    void setValSpeed(double angle);

    /*!
     * \brief Установка параметров треугольника
     * \param width - ширина основания
     * \param length - длина треугольника
     * \param color - цвет треугольника
     */
    void setParameters(double width, double length, QColor color);


private:
    QImage   img_; ///< картинка

    /// Параметры треугольника ограничения спидометра
    double widthTriangle_;  ///< ширина основания
    double lengthTriangle_; ///< длина
    QColor colorTriangle_;  ///< цвет



};




#endif // STRIANGLE_H
