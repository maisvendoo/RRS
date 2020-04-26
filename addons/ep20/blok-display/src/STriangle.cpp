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

#include "STriangle.h"
#include "qmath.h"


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
STriangle::STriangle(QWidget *parent) : QLabel(parent)
{
    this->resize( parent->size() );

    // инициализация (параметры треугольника по умолчанию)
    widthTriangle_  = 6.0;
    lengthTriangle_ = 20.0;
    colorTriangle_  = QColor(255,255,255);


    img_ = QImage( this->size(),
                   QImage::Format_ARGB32_Premultiplied );


}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
STriangle::~STriangle()
{

}

//-----------------------------------------------------------------------------
//  Вращение треугольника
//-----------------------------------------------------------------------------
void STriangle::setValSpeed(double angle)
{
    img_.fill(Qt::transparent);

    QPainter paint(&img_);
    // указываем цвет границы
    paint.setPen(QPen(QColor(colorTriangle_), 1, Qt::SolidLine));
    // указываем цвет заливки
    paint.setBrush(QColor(colorTriangle_));
    // повышаем качество рисовки
    paint.setRenderHint(QPainter::Antialiasing, true);



              angle = qDegreesToRadians(angle);
    double fooAngle = qDegreesToRadians(90.0);
            // положение середины основания треугольника
    double  x0  = this->width()/2.0  + ((this->width() - 50)/2.0)*cos(angle) + lengthTriangle_*cos(angle),
            // положение середины основания треугольника
            y0  = this->height()/2.0 + ((this->height() - 50)/2.0)*sin(angle) + lengthTriangle_*sin(angle),
            // длина треугольника
            r  = -lengthTriangle_,
            // половина ширины основания треугольника
            r2 = widthTriangle_;

    QPolygonF triangle;
    triangle << QPointF( x0 + r*cos(angle),
                         y0 + r*sin(angle) )
             << QPointF( x0 + r2*cos(angle+fooAngle),
                         y0 + r2*sin(angle+fooAngle) )
             << QPointF( x0 + r2*cos(angle-fooAngle),
                         y0 + r2*sin(angle-fooAngle) );


    paint.drawPolygon(triangle);
    paint.end();

    this->setPixmap(QPixmap::fromImage(img_));

}

//-----------------------------------------------------------------------------
//  Установка параметров треугольника
//-----------------------------------------------------------------------------
void STriangle::setParameters(double width, double length, QColor color)
{
    this->widthTriangle_ = width;
    this->lengthTriangle_ = length;
    this->colorTriangle_ = color;
}





