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
    painter.fillRect(rect(), background_color);

    // Траектории маршрута
    if (route_begin_trajectory && route_trajectories.empty())
    {
        drawTrajectoryHighlight(painter, route_begin_trajectory, QColor(255, 0, 0));
    }
    for (auto& traj : route_trajectories)
    {
        drawTrajectoryHighlight(painter, traj, QColor(0, 255, 0));
    }

    // Ближайшая к курсору траетория
    if (nearest_trajectory != route_begin_trajectory)
    {
        drawTrajectoryHighlight(painter, nearest_trajectory, QColor(0, 255, 255));
    }

    painter.end();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BackGroundWidget::drawTrajectoryHighlight(QPainter &painter, Trajectory *traj, QColor highlight)
{
    if (!traj)
    {
        return;
    }

    float max_width = std::ceil(static_cast<float>(scale)) + 3.0f;
    float width = max_width;
    std::vector<QPen> higlight_pens;
    while (width >= 0.0f)
    {
        QPen pen;
        pen.setWidth(width * 2.0f + 3.0f);
        pen.setColor(mix_color(background_color, highlight, (width / max_width)));
        pen.setCapStyle(Qt::FlatCap);
        higlight_pens.push_back(pen);
        width -= 1.0f;
    }

    for (auto& track : traj->getTracks())
    {
        QPoint p0 = coord_transform(track.begin_point);
        QPoint p1 = coord_transform(track.end_point);

        for (auto& pen : higlight_pens)
        {
            painter.setPen(pen);
            painter.drawLine(p0, p1);
        }
    }
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QColor BackGroundWidget::mix_color(QColor color1, QColor color2, float mix_ratio)
{
    if (mix_ratio <= 0.0)
    {
        return color2;
    }

    if (mix_ratio >= 1.0)
    {
        return color1;
    }

    QColor result;
    result.setRedF  (color1.redF()   * mix_ratio + color2.redF()   * (1.0f - mix_ratio));
    result.setGreenF(color1.greenF() * mix_ratio + color2.greenF() * (1.0f - mix_ratio));
    result.setBlueF (color1.blueF()  * mix_ratio + color2.blueF()  * (1.0f - mix_ratio));
    return result;
}
