//-----------------------------------------------------------------------------
//
//      Элемент блока информации (треугольник)
//      (c) РГУПС, ВЖД 14/04/2017
//      Разработал: Даглдиян Б.Д.
//
//-----------------------------------------------------------------------------
/*!
 * \file
 * \brief Класс элемента блока информации (треугольник)
 * \copyright РГУПС, ВЖД
 * \author Даглдиян Б.Д.
 * \date 14/04/2017
 */

#include "InformPartTriangle.h"
#include <QtCore/qmath.h>


//-----------------------------------------------------------------------------
//  Конструктор. Рисуем элементы информационного блока
//-----------------------------------------------------------------------------
InformPartTriangle::InformPartTriangle(QPoint pos, int size, QColor color,
                                        QWidget *parent) : QLabel(parent)
{
    colorTriangle_ = color;

    // устанавливаем геометрию класса
    this->setGeometry(pos.x(), pos.y(), size, size*0.8);


//    labelTriangle_ = new QLabel("", this);
//    // задаем геометрию
//    labelTriangle_->setGeometry( 0,
//                                 0,
//                                 this->width(),
//                                 this->height() );

    // создаем img с размером label
    imgTriangle_ = QImage( this->size(),
                           QImage::Format_ARGB32_Premultiplied );

    setTriangle(0, false, false);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
InformPartTriangle::~InformPartTriangle()
{

}

//-----------------------------------------------------------------------------
//  Прорисовка треугольника
//-----------------------------------------------------------------------------
void InformPartTriangle::setTriangle(int angle, bool isBrush,
                                     bool drawCircle, bool drawStr)
{
    imgTriangle_.fill(Qt::transparent);

    QPainter paint(&imgTriangle_);
    paint.setPen(QPen(colorTriangle_,1,Qt::SolidLine));
    paint.setRenderHint(QPainter::Antialiasing, true);


    if (isBrush)
    {
        paint.setBrush(colorTriangle_);
        drawTriangle_(paint, 1, 1);
    }
    else
    {
        int j = 0;
        for (int i = 1; i <= 5; i++)
        {
            if (i % 2 == 0)
                j++;

            drawTriangle_(paint, i, j);
        }
    }

    if (drawCircle)
        drawCircle_(paint);

    if (drawStr)
        drawText(paint, "ТСКБМ");

    paint.end();

    this->setPixmap(QPixmap::fromImage(imgTriangle_)
                              .transformed( QTransform().rotate(angle),
                                            Qt::SmoothTransformation) );

}

//-----------------------------------------------------------------------------
//  Нарисовать треугольник
//-----------------------------------------------------------------------------
void InformPartTriangle::drawTriangle_(QPainter &paint, int i, int j)
{    
    QPolygon triangle;
    triangle << QPoint(i, j)
             << QPoint(this->width()-i, j)
             << QPoint(this->width()/2, this->height() - i);
    paint.drawPolygon(triangle);
}

//-----------------------------------------------------------------------------
//  Нарисовать круг в треугольнике
//-----------------------------------------------------------------------------
void InformPartTriangle::drawCircle_(QPainter &paint)
{
    // 1. указываем цвет, толщину и стиль линии
    paint.setPen(QPen( QColor(255,0,0),
                       this->width()/4,
                       Qt::SolidLine,
                       Qt::RoundCap ));
    // рисуем круг
    paint.drawPoint(this->width()/2, this->height()*0.4);   // (1-1/qSqrt(3))*
}

//-----------------------------------------------------------------------------
//  Нарисовать текст в треугольнике
//-----------------------------------------------------------------------------
void InformPartTriangle::drawText(QPainter &paint, QString str)
{
    paint.setPen(QPen(QColor(255,255,255)));
    paint.setFont(QFont("Arial",10,87));
    paint.drawText( QRect( this->width()/2-str.size()*4.5,
                           this->height()/5,
                           this->width(),
                           this->height() ),
                    str);
}
