#include    "background-widget.h"
#include    <QPainter>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BackGroundWidget::BackGroundWidget(QWidget *parent) : QWidget(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BackGroundWidget::~BackGroundWidget()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BackGroundWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.begin(this);

    // Серый фон
    painter.fillRect(rect(), QColor(150, 150, 150));

    if (nearest_trajectory == nullptr)
    {
        painter.end();
        return;
    }

    // Подсветка фона вдоль ближайшей к курсору траектории
    QPen pen_wide;
    pen_wide.setWidth((scale >= 2) ? 9 : 7);
    pen_wide.setColor(QColor(100, 196, 196));
    QPen pen_mid;
    pen_wide.setWidth((scale >= 2) ? 7 : 5);
    pen_wide.setColor(QColor(50, 222, 222));
    QPen pen_tight;
    pen_tight.setWidth(3);
    pen_tight.setColor(QColor(0, 255, 255));

    for (auto& track : nearest_trajectory->getTracks())
    {
        QPoint p0 = coord_transform(track.begin_point);
        QPoint p1 = coord_transform(track.end_point);

        painter.setPen(pen_wide);
        painter.drawLine(p0, p1);
        painter.setPen(pen_mid);
        painter.drawLine(p0, p1);
        painter.setPen(pen_tight);
        painter.drawLine(p0, p1);
    }
    painter.end();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QPoint BackGroundWidget::coord_transform(dvec3 point)
{
    QPoint p;

    // У маршрутов направление вперёд в основном по оси Y,
    // отрисовываем его слева направо - по оси X виджета,
    // т.е. меняем оси местами
    p.setX(this->width() / 2 + shift.x() + scale * point.y);
    p.setY(this->height() / 2 + shift.y() + scale * point.x);

    return p;
}
