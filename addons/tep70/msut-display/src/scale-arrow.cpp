#include "scale-arrow.h"
#include    <QPainter>
#include    <QtCore/qmath.h>



ScaleArrow::ScaleArrow(QSize _size, int otstupSverhu, QWidget *parent)
    : QLabel(parent)
    , isArrow_(false)
    , maxVal_(4)
{
    this->resize(_size);
    w_2_ = this->width()/2.0;
    h_2_ = this->height()/2.0;

    cpX_ = this->width()/2.0 - 1 ;
    //cpY_ = this->height()*3/4.0;
    cpY_ = this->height() + 28*1.5;
    cpY_ = this->width()/2.0 + otstupSverhu;

    img_ = QImage(this->size(), QImage::Format_ARGB32_Premultiplied);
    //draw_(60);

}

void ScaleArrow::setIsArrow(bool flag)
{
    this->isArrow_ = flag;
}

void ScaleArrow::setMaxVal(double val)
{
    this->maxVal_ = val;
}

void ScaleArrow::setVal(double val)
{
//    if (val < 0) val = 0;
//    if (val > maxVal_) val = maxVal_;

    draw_(val);
}



void ScaleArrow::draw_(double _val)
{
    img_.fill(Qt::transparent);
    QPixmap pix = QPixmap::fromImage(img_);
    QPainter paint(&pix);
    paint.setRenderHint(QPainter::Antialiasing, true);

    int sgp_angleArcEnd = 45; // +++
    //int sgp_maxSpeedScale = 60; // maxVal_
    //int sgp_lenArcScale = 90; // +++
    double stepDeg_ = (360.0-270)/maxVal_;



    //double angleInDeg = (360.0-sgp_angleArcEnd) - 90;
    double angleInDeg = (360.0-sgp_angleArcEnd) - (maxVal_ - _val)*stepDeg_;




    double angle = 0;

    angle = qDegreesToRadians(angleInDeg);
    double fooAngle = qDegreesToRadians(90.0);
    // длина стрелки
    double r  = 1;
    // половина ширины основания стрелки
    double r2 = 1.5;

    double r2Lk = 0.66;

    if (isArrow_)
    {
    QPolygonF triangle;
    triangle << QPointF( cpX_ + (w_2_*r)*cos(angle),
                         cpY_ + (w_2_*r)*sin(angle) )
             << QPointF( cpX_ + (w_2_*r2Lk*r)*cos(angle) + r2*cos(angle+fooAngle),
                         cpY_ + (w_2_*r2Lk*r)*sin(angle) + r2*sin(angle+fooAngle) )
             << QPointF( cpX_ + (w_2_*r2Lk*r)*cos(angle) + r2*cos(angle-fooAngle),
                         cpY_ + (w_2_*r2Lk*r)*sin(angle) + r2*sin(angle-fooAngle) );

    paint.setPen(QPen(Qt::black, 1, Qt::SolidLine));
    paint.setBrush(Qt::red);
    paint.drawPolygon(triangle);
    }
    else
    {
        r = 1.02;
        r2Lk = 0.74;
        paint.setPen(QPen(Qt::red, 1, Qt::SolidLine));
        paint.drawLine(cpX_ + (w_2_*r)*cos(angle),
                       cpY_ + (w_2_*r)*sin(angle),
                       cpX_ + (w_2_*r2Lk*r)*cos(angle),
                       cpY_ + (w_2_*r2Lk*r)*sin(angle));
    }


//    paint.setPen(QPen(Qt::green, 2, Qt::SolidLine));
//    QRect rect(0,/*28*/40, this->width(), this->width());
//    paint.drawArc(rect, sgp_angleArcEnd*16, sgp_lenArcScale*16);

    paint.end();
    this->setPixmap(pix);
}




// +++++++++++++++++++++++++++++++
//void ScaleArrow::draw_(int _val)
//{
//    img_.fill(Qt::transparent);
//    QPixmap pix = QPixmap::fromImage(img_);
//    QPainter paint(&pix);
//    paint.setRenderHint(QPainter::Antialiasing, true);

//    int sgp_angleArcEnd = 45; // +++
//    int sgp_maxSpeedScale = 60;
//    int sgp_lenArcScale = 90; // +++
//    double stepDeg_ = (360.0-270)/sgp_maxSpeedScale;



//    //double angleInDeg = (360.0-sgp_angleArcEnd) - 90;
//    double angleInDeg = (360.0-sgp_angleArcEnd) - (sgp_maxSpeedScale - _val)*stepDeg_;


//    paint.setPen(QPen(Qt::black, 1, Qt::SolidLine));
//    paint.setBrush(Qt::red);

//    double angle = 0;

//    angle = qDegreesToRadians(angleInDeg);
//    double fooAngle = qDegreesToRadians(90.0);
//    // длина стрелки
//    double r  = 1;
//    // половина ширины основания стрелки
//    double r2 = 1.5;

//    double r2Lk = 0.66;

//    QPolygonF triangle;
//    triangle << QPointF( cpX_ + (w_2_*r)*cos(angle),
//                         cpY_ + (w_2_*r)*sin(angle) )
//             << QPointF( cpX_ + (w_2_*r2Lk*r)*cos(angle) + r2*cos(angle+fooAngle),
//                         cpY_ + (w_2_*r2Lk*r)*sin(angle) + r2*sin(angle+fooAngle) )
//             << QPointF( cpX_ + (w_2_*r2Lk*r)*cos(angle) + r2*cos(angle-fooAngle),
//                         cpY_ + (w_2_*r2Lk*r)*sin(angle) + r2*sin(angle-fooAngle) );

//    paint.drawPolygon(triangle);


//    paint.setPen(QPen(Qt::green, 2, Qt::SolidLine));
//    QRect rect(0,28, this->width(), this->width());
//    paint.drawArc(rect, sgp_angleArcEnd*16, sgp_lenArcScale*16);

//    paint.end();
//    this->setPixmap(pix);
//}


