#include "status-bar.h"

#include <QPainter>

#include <QDebug>



StatusBar::StatusBar(QSize _size, int maxVal, double EPS, QWidget *parent)
    : QLabel(parent)
    , maxVal_(maxVal)
    , oldVal_(-1)
    , EPS_(EPS)
{
    this->resize(_size);
    //this->setStyleSheet("border: 1px solid green;");

    img_ = QImage(this->size(), QImage::Format_ARGB32_Premultiplied);

}



double StatusBar::setVal(double val)
{
    if (val < 0) val = 0;
    if (val > maxVal_) val = maxVal_;

    if (std::abs(val - oldVal_) < EPS_)
        return -1.0;

    drawBar_(val);

    oldVal_ = val;

    return  val;
}



void StatusBar::drawBar_(double val)
{
    img_.fill(Qt::transparent);
    QPixmap pix = QPixmap::fromImage(img_);
    QPainter paint(&pix);


    //paint.setPen(QColor("yellow"));
    paint.setPen(Qt::transparent);
    paint.setBrush(QColor("red"));


    int valX = this->width() * val / maxVal_;

    QPolygon p;
    p << QPoint(0,0)
      << QPoint(valX, 0)
      << QPoint(valX, this->height()-10)
      << QPoint(0, this->height()-10);

    paint.drawPolygon(p);


    double linesDeltaX = this->width() / 10.0;

    for (int i = 1; i < 10 ; ++i)
    {
        paint.setPen(QColor(70,70,70));

        int fooX = i*linesDeltaX;
        paint.drawLine(fooX, 0,
                       fooX, this->height()-11);

    }


    paint.end();
    this->setPixmap(pix);

}
