#include "reverse-indication.h"

#include <QPainter>

const QPointF indForward = {4.5, 4.5};
const QPointF indBackward = {20.5, 4.5};
const int     indSize = 7;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ReverseInd::ReverseInd(QSize _size, QWidget *parent)
    : QLabel(parent)
{
    this->resize(_size);
    //this->setStyleSheet("border: 1px solid red;");

    QPixmap pix = QPixmap(this->size());
    pix.fill(Qt::transparent);
    QPainter paint(&pix);
    paint.setRenderHint(QPainter::Antialiasing, true);
    paint.setPen(QPen( QColor(Qt::green),
                       indSize,
                       Qt::SolidLine,
                       Qt::RoundCap ));

    paint.drawPoint(indForward);
    paint.drawPoint(indBackward);
    paint.end();
    this->setPixmap(pix);
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ReverseInd::setReverse(int val)
{
    if (val == oldVal_)
        return;

    oldVal_ = val;

    if (val == 0)
    {
        this->setPixmap(QPixmap());
        return;
    }

    drawReverse_(val);
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ReverseInd::drawReverse_(int val)
{
    QPixmap pix = QPixmap(this->size());
    pix.fill(Qt::transparent);
    QPainter paint(&pix);
    paint.setRenderHint(QPainter::Antialiasing, true);
    paint.setPen(QPen( QColor(Qt::green),
                       indSize,
                       Qt::SolidLine,
                       Qt::RoundCap ));


    if (val == 1)
        paint.drawPoint(indForward);

    if (val == -1)
        paint.drawPoint(indBackward);


    paint.end();
    this->setPixmap(pix);
}
