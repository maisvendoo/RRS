#include    "background-widget.h"
#include    <switch.h>
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
    painter.fillRect(rect(), color_background);

    // Траектории маршрута
    if (route_begin_trajectory && route_trajectories.empty())
    {
        drawTrajectoryHighlight(painter, route_begin_trajectory, color_no_route);
    }
    for (auto& traj : route_trajectories)
    {
        drawTrajectoryHighlight(painter, traj, color_route);
    }

    // Ближайшая к курсору стрелка
    if (!drawSwitchHighlight(painter, nearest_switch, nearest_switch_dir, color_nearest))
    {
        // Или ближайшая к курсору траетория
        if (nearest_trajectory != route_begin_trajectory)
        {
            drawTrajectoryHighlight(painter, nearest_trajectory, color_nearest);
        }
    }

    painter.end();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool BackGroundWidget::drawTrajectoryHighlight(QPainter &painter, Trajectory *traj, QColor highlight)
{
    if (!traj)
    {
        return false;
    }

    float max_width = std::ceil(static_cast<float>(scale)) + 3.0f;
    float width = max_width;
    std::vector<QPen> higlight_pens;
    while (width >= 0.0f)
    {
        QPen pen;
        pen.setWidth(width * 2.0f + 3.0f);
        pen.setColor(mix_color(color_background, highlight, (width / max_width)));
        pen.setCapStyle(Qt::RoundCap);
        higlight_pens.push_back(pen);
        width -= 1.0f;
    }

    for (auto& pen : higlight_pens)
    {
        painter.setPen(pen);
        for (const auto& track : traj->getTracks())
        {
            QPoint p0 = coord_transform(track.begin_point);
            QPoint p1 = coord_transform(track.end_point);
            painter.drawLine(p0, p1);
        }
    }
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool BackGroundWidget::drawSwitchHighlight(QPainter& painter, Switch* conn, std::int8_t dir, QColor highlight)
{
    Switch* sw = dynamic_cast<Switch*>(conn);
    if (!sw || (dir == 0))
    {
        return false;
    }

    Trajectory* traj_plus = (dir < 0) ? sw->trajectories[SW_BWD_PLUS] : sw->trajectories[SW_FWD_PLUS];
    Trajectory* traj_minus = (dir < 0) ? sw->trajectories[SW_BWD_MINUS] : sw->trajectories[SW_FWD_MINUS];

    if ((traj_plus == nullptr) || (traj_minus == nullptr))
    {
        return false;
    }

    float max_width = std::ceil(static_cast<float>(scale)) + 3.0f;
    float width = max_width;
    std::vector<QPen> higlight_pens;
    while (width >= 0.0f)
    {
        QPen pen;
        pen.setWidth(width * 2.0f + 3.0f);
        pen.setColor(mix_color(color_background, highlight, (width / max_width)));
        pen.setCapStyle(Qt::RoundCap);
        higlight_pens.push_back(pen);
        width -= 1.0f;
    }

    for (auto& pen : higlight_pens)
    {
        painter.setPen(pen);

        for (Trajectory* traj : {traj_plus, traj_minus})
        {
            double draw_len = traj->getLength() - 1.0;

            if (dir > 0)
            {
                dir_t dir_fwd = FWD;
                if (Switch* next_sw = traj->getNextSwitch(dir_fwd))
                {
                    Switch_state_t next_state = (dir_fwd == FWD) ? next_sw->getStateBwd() : next_sw->getStateFwd();
                    if (   (next_state != NO_POSSIBLE_DIRECTION)
                        && (next_state != ONLY_MINUS)
                        && (next_state != ONLY_PLUS))
                    {
                        draw_len = traj->getLength() * 0.5;
                    }
                }
                draw_len = std::min(draw_len, switch_length);

                size_t i = 0;
                dvec3 fwd = traj->getTracks().begin()->begin_point;
                QPoint fwd_point = coord_transform(fwd);
                const track_t* track_next = nullptr;
                while (draw_len > 0.0)
                {
                    track_next = &traj->getTracks().at(i);
                    fwd += track_next->orth * std::min(draw_len, track_next->len);
                    QPoint fwd_point_next = coord_transform(fwd);
                    painter.drawLine(fwd_point, fwd_point_next);

                    fwd_point = fwd_point_next;
                    draw_len = draw_len - track_next->len;
                    ++i;
                }
            }
            else
            {
                dir_t dir_bwd = BWD;
                if (Switch* next_sw = traj->getNextSwitch(dir_bwd))
                {
                    Switch_state_t next_state = (dir_bwd == BWD) ? next_sw->getStateFwd() : next_sw->getStateBwd();
                    if (   (next_state != NO_POSSIBLE_DIRECTION)
                        && (next_state != ONLY_MINUS)
                        && (next_state != ONLY_PLUS))
                    {
                        draw_len = traj->getLength() * 0.5;
                    }
                }
                draw_len = std::min(draw_len, switch_length);

                size_t i = 1;
                dvec3 bwd = (traj->getTracks().end() - i)->end_point;
                QPoint bwd_point = coord_transform(bwd);
                const track_t* track_next = nullptr;
                while (draw_len > 0.0)
                {
                    track_next = &traj->getTracks().at(traj->getTracks().size() - i);
                    bwd -= track_next->orth * std::min(draw_len, track_next->len);
                    QPoint bwd_point_next = coord_transform(bwd);
                    painter.drawLine(bwd_point, bwd_point_next);

                    bwd_point = bwd_point_next;
                    draw_len = draw_len - track_next->len;
                    ++i;
                }
            }
        }
    }
    return true;
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
