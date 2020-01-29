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

#include "SScaleMajorLine.h"
#include <QtCore/qmath.h>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
SScaleMajorLine::SScaleMajorLine( int len, int width, QColor color, int col,
                                  double angleStep, double angleZeroPoint,
                                  int arcLimitWidth,
                                  QWidget *parent ) : QLabel(parent)
{
    this->resize( parent->size() );

    w_2_ = this->width()/2.0;
    h_2_ = this->height()/2.0;

    this->lineLength_ = len;
    this->lineWidth_ = width;
    this->lineColor_ = color;
    this->lineCount_ = col;
    this->angleStep_ = angleStep;
    this->aZeroPoint_ = angleZeroPoint;
    this->arcLimitWidth_ = arcLimitWidth;

    // создаем img размером с widget
    img_ = QImage( this->size(),
                   QImage::Format_ARGB32_Premultiplied );

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
SScaleMajorLine::~SScaleMajorLine()
{

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void SScaleMajorLine::drawMajorLine()
{
    img_.fill(Qt::transparent);
    // создаем и загружаем pixmap с картинки img_
    QPixmap pix = QPixmap::fromImage(img_);
    // передаем ссылку на pixmap в QPainter
    QPainter paint(&pix);
    // повышаем качество рисовки
    paint.setRenderHint(QPainter::Antialiasing, true);


    for (int i = 0; i <= lineCount_; i++)
    {
        double angle = i*angleStep_ - aZeroPoint_;
        angle = qDegreesToRadians(angle);

        // радиус круга
        double r  = - 28 + arcLimitWidth_/2;
        // радиус "внутреннего" круга
        double r2  = - 28 + arcLimitWidth_/2 - lineLength_;

        // 1. указываем цвет, толщину и стиль линии
        paint.setPen(QPen( QColor(lineColor_),
                           lineWidth_,
                           Qt::SolidLine,
                           Qt::RoundCap ));

        // 2. рисуем линию
        paint.drawLine( w_2_ + (w_2_ + r)*cos(angle),
                        h_2_ + (h_2_ + r)*sin(angle),
                        w_2_ + (w_2_ + r2)*cos(angle),
                        h_2_ + (h_2_ + r2)*sin(angle) );

    }


    paint.end();

    this->setPixmap(pix);
}

