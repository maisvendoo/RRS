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

    paint.setPen(QPen(QColor("#000080"), 1, Qt::SolidLine));

    QLinearGradient lineGr;
    lineGr.setStart(this->width(), 0);
    lineGr.setFinalStop(this->width(), this->height());
    lineGr.setColorAt(0.0, QColor("#000080"));
    lineGr.setColorAt(0.1, QColor("#0000a0"));
    lineGr.setColorAt(0.4, QColor("#4040a9"));
    lineGr.setColorAt(0.7, QColor("#000080"));
    lineGr.setColorAt(1.0, QColor("#000080"));
    paint.setBrush(lineGr);


    QPolygonF p1;
    p1 << QPointF(0, 0)
       << QPointF(this->width()-2, 0)
       << QPointF(this->width()-2 - 5, this->height())
       << QPointF(0, this->height());

    paint.drawPolygon(p1);


    paint.end();
    this->setPixmap(pix);
}
