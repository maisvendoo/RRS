#ifndef     BACKGROUND_WIDGET_H
#define     BACKGROUND_WIDGET_H

#include    <QWidget>

#include    <trajectory.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class BackGroundWidget : public QWidget
{
    Q_OBJECT

public:

    BackGroundWidget(QWidget *parent = nullptr);

    ~BackGroundWidget();

    Trajectory* nearest_trajectory = nullptr;

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

    /// Серый фон
    static constexpr QColor background_color = QColor(150, 150, 150);

    /// Масштаб отображения карты
    double scale = 1.0;

    /// Текущее смещение координат
    QPoint shift = {0, 0};

    void paintEvent(QPaintEvent *event);

    void drawTrajectoryHighlight(QPainter& painter, Trajectory* traj, QColor highlight);

    QPoint coord_transform(dvec3 point);

    QColor mix_color(QColor color1, QColor color2, float mix_ratio);
};

#endif // BACKGROUND_WIDGET_H
