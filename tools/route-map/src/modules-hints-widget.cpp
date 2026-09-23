#include    <modules-hints-widget.h>

#include    <topology-trajectory-device.h>

#include    <QPainter>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ModulesHintsWidget::ModulesHintsWidget(QWidget *parent) : QWidget(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ModulesHintsWidget::~ModulesHintsWidget()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModulesHintsWidget::paintEvent(QPaintEvent *event)
{
    (void)event;

    if ((traj_list == nullptr) || menu_view_topology_modules.isEmpty())
    {
        return;
    }

    QPainter painter;
    painter.begin(this);

    for (auto& traj : *traj_list)
    {
        drawTrajectoryModules(traj, painter);
    }

    painter.end();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ModulesHintsWidget::drawTrajectoryModules(Trajectory *traj, QPainter &painter)
{
    for (TrajectoryDevice* device : traj->getTrajectoryDevices())
    {
        QString module_name = device->getName();
        QAction* action_view_module = menu_view_topology_modules.value(module_name, nullptr);
        if (action_view_module && action_view_module->isChecked())
        {
            std::vector<draw_line_t> lines;
            std::vector<draw_circle_t> circles;
            device->getDrawElements(lines, circles, scale);

            for (auto& line : lines)
            {
                line.color.r = std::clamp(line.color.r, 0.0f, 1.0f) * 255.0f;
                line.color.g = std::clamp(line.color.g, 0.0f, 1.0f) * 255.0f;
                line.color.b = std::clamp(line.color.b, 0.0f, 1.0f) * 255.0f;

                QPen pen;
                pen.setColor(QColor(int(line.color.r), int(line.color.g), int(line.color.b)));
                pen.setWidth(line.width);
                pen.setCapStyle(Qt::FlatCap);
                painter.setPen(pen);

                QPoint p0 = coord_transform(line.begin_point);
                QPoint p1 = coord_transform(line.end_point);

                painter.drawLine(p0, p1);
            }

            for (auto& circle : circles)
            {
                circle.color.r = std::clamp(circle.color.r, 0.0f, 1.0f) * 255.0f;
                circle.color.g = std::clamp(circle.color.g, 0.0f, 1.0f) * 255.0f;
                circle.color.b = std::clamp(circle.color.b, 0.0f, 1.0f) * 255.0f;
                painter.setBrush(QColor(int(circle.color.r), int(circle.color.g), int(circle.color.b)));

                QPoint point = coord_transform(circle.point);

                painter.drawEllipse(point, circle.radius, circle.radius);
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QPoint ModulesHintsWidget::coord_transform(dvec3 point)
{
    QPoint p;

    p.setX(this->width() / 2 + shift.x() + scale * point.y);
    p.setY(this->height() / 2 + shift.y() + scale * point.x);

    return p;
}
