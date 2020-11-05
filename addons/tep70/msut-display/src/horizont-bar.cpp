#include "horizont-bar.h"

#include    <QPainter>


HorizontBar::HorizontBar(QSize _size, QWidget *parent)
    : QLabel(parent)
{
    this->resize(_size);

    img_ = QImage(this->size(), QImage::Format_ARGB32_Premultiplied);
    draw_();

}

void HorizontBar::draw_()
{
    img_.fill(Qt::transparent);
    QPixmap pix = QPixmap::fromImage(img_);
    QPainter paint(&pix);
    paint.setRenderHint(QPainter::Antialiasing, true);

    paint.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
    paint.setBrush(Qt::blue);


    QPolygonF p1;
    p1 << QPointF(0, 0)
       << QPointF(this->width()-2, 0)
       << QPointF(this->width()-2 - 5, this->height())
       << QPointF(0, this->height());

    paint.drawPolygon(p1);


    paint.end();
    this->setPixmap(pix);
}
