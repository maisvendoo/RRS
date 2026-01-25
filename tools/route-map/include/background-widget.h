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

    Trajectory* route_begin_trajectory = nullptr;

    std::vector<Trajectory*> route_trajectories;

    Connector* nearest_switch = nullptr;

    std::int8_t nearest_switch_dir = 0;

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

    void setSwitchLength(double value)
    {
        switch_length = value;
    }

private:

    /// Серый фон
    static constexpr QColor color_background = QColor(150, 150, 150);
    /// Подсветка ближайшего к курсору
    static constexpr QColor color_nearest = QColor(0, 255, 255);
    /// Подсветка траекторий маршрута
    static constexpr QColor color_route = QColor(0, 255, 0);
    /// Подсветка траектории начала маршрута, если путь не найден
    static constexpr QColor color_no_route = QColor(255, 0, 0);

    /// Масштаб отображения карты
    double scale = 1.0;

    /// Текущее смещение координат
    QPoint shift = {0, 0};

    /// Длина отрисовки выбранной траектории стрелки, м
    double switch_length = 35.0;

    void paintEvent(QPaintEvent *event);

    bool drawTrajectoryHighlight(QPainter& painter, Trajectory* traj, QColor highlight);

    bool drawSwitchHighlight(QPainter& painter, Connector* conn, std::int8_t dir, QColor highlight);

    QPoint coord_transform(dvec3 point);

    QColor mix_color(QColor color1, QColor color2, float mix_ratio);
};

#endif // BACKGROUND_WIDGET_H
