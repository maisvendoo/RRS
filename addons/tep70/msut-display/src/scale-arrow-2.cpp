#include "scale-arrow-2.h"
#include    <QPainter>
#include    <QtCore/qmath.h>



ScaleArrow2::ScaleArrow2(QSize _size, int otstupSverhu, QWidget *parent)
    : QLabel(parent)
    , maxVal_(4)
{
    this->resize(_size);
    w_2_ = this->width()/2.0;
    h_2_ = this->width()/2.0 + 2;

    cpX_ = this->width()/2.0 + 20;
    cpY_ = this->width()/2.0;
//    //cpY_ = this->height()*3/4.0;
//    cpY_ = this->height() + 28*1.5;
//    cpY_ = this->width()/2.0 + otstupSverhu;

    img_ = QImage(this->size(), QImage::Format_ARGB32_Premultiplied);
    //draw_(60);

}



void ScaleArrow2::setMaxVal(double val)
{
    this->maxVal_ = val;
}



void ScaleArrow2::setVal(double val)
{
//    if (val < 0) val = 0;
//    if (val > maxVal_) val = maxVal_;

    draw_(val);
}



void ScaleArrow2::draw_(double _val)
{
    img_.fill(Qt::transparent);
    QPixmap pix = QPixmap::fromImage(img_);
    QPainter paint(&pix);
    paint.setRenderHint(QPainter::Antialiasing, true);

    int sgp_angleArcEnd = -40; // +++
    //int sgp_maxSpeedScale = 60; // maxVal_
    int sgp_lenArcScale = 260; // +++
    double stepDeg_ = /*(360.0-90)*/260/maxVal_;



    //double angleInDeg = (360.0-sgp_angleArcEnd) - 90;
    double angleInDeg = (360.0-sgp_angleArcEnd) - (maxVal_ - _val)*stepDeg_;




    double angle = 0;

    angle = qDegreesToRadians(angleInDeg);
    double fooAngle = qDegreesToRadians(90.0);
    // длина стрелки
    double r  = 0.8;
    // половина ширины основания стрелки
    double r2 = 3.5;


    QPolygonF triangle;
    triangle << QPointF( w_2_ + (w_2_*r)*cos(angle),
                         h_2_ + (h_2_*r)*sin(angle) )
             << QPointF( w_2_ + r2*cos(angle+fooAngle),
                         h_2_ + r2*sin(angle+fooAngle) )
             << QPointF( w_2_ + r2*cos(angle-fooAngle),
                         h_2_ + r2*sin(angle-fooAngle) );

    paint.setPen(QPen(Qt::red, 1, Qt::SolidLine));
    paint.setBrush(Qt::red);
    paint.drawPolygon(triangle);


    paint.setPen(QPen( QColor(Qt::red),
                       16,
                       Qt::SolidLine,
                       Qt::RoundCap ));
    paint.drawPoint(w_2_, h_2_);


//    paint.setPen(QPen(Qt::green, 2, Qt::SolidLine));
//    QRect rect(10,10, this->width()-20, this->width()-20);
//    paint.drawArc(rect, sgp_angleArcEnd*16, sgp_lenArcScale*16);

    paint.end();
    this->setPixmap(pix);
}


