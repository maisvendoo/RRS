#ifndef     MODULES_HINTS_WIDGET_H
#define     MODULES_HINTS_WIDGET_H

#include    <QWidget>

#include    <trajectory.h>
#include    <topology-types.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ModulesHintsWidget : public QWidget
{
    Q_OBJECT

public:

    ModulesHintsWidget(QWidget *parent = nullptr);

    ~ModulesHintsWidget();

    traj_list_t *traj_list = nullptr;

    QMap<QString, QAction*> menu_view_topology_modules;

    void resize(int width, int height)
    {
        QWidget::resize(width, height);
    }

    void setScale(double map_scale)
    {
        scale = map_scale;
    }

    void setShift(QPoint map_shift)
    {
        shift = map_shift;
    }

private:

    /// Масштаб отображения карты
    double scale = 1.0;

    /// Текущее смещение координат
    QPoint shift = {0, 0};

    void paintEvent(QPaintEvent *event);

    void drawTrajectoryModules(Trajectory *traj, QPainter &painter);

    QPoint coord_transform(dvec3 point);
};

#endif // MODULES_HINTS_WIDGET_H
