#ifndef     MAP_WIDGET_H
#define     MAP_WIDGET_H

#include    <QMap>
#include    <topology-types.h>
#include    <trajectory.h>
#include    <simulator-update-struct.h>
#include    <switch-label.h>
#include    <signals-data-types.h>
#include    <signal-label.h>
#include    <train-label.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MapWidget : public QWidget
{
    Q_OBJECT

public:

    MapWidget(QWidget *parent = nullptr);

    ~MapWidget();

    traj_list_t *traj_list = nullptr;

    conn_list_t *conn_list = nullptr;

    Trajectory* nearest_trajectory = nullptr;

    Connector* nearest_connector = nullptr;

    simulator_update_players_t *players_data = nullptr;

    simulator_update_pos_t *train_data = nullptr;

    std::vector<double> *vehicles_half_length = nullptr;

    topology_stations_list_t *stations = nullptr;

    QMap<QString, SwitchLabel *> switch_labels;

    signals_data_t *signals_data = nullptr;

    QMap<QString, SignalLabel *> signal_labels_fwd;

    QMap<QString, SignalLabel *> signal_labels_bwd;

    QMap<QString, QLabel *> traj_labels;

    std::vector<TrainLabel *> train_labels;

    void resize(int width, int height);

    void setSwitchLength(double value);

    void setSignalRadius(double value);

    void setSignalOffset(double value);

    double getScale() const
    {
        return scale;
    }

    QPoint getShift() const
    {
        return map_shift;
    }

    void showTrajNames(bool is_show)
    {
        show_traj_names = is_show;
    }

signals:

    void sigOpenTrajectoryMenu(Trajectory* nearest_traj);

public slots:

    void slotStationAtCenter(int idx);

    void slotPlayerAtCenter(int idx);

private:

    /// Масштаб отображения карты
    double scale = 1.0;

    /// Шаг увеличения масштаба
    double scale_inc_step_coeff = sqrt(2.0);
    /// Шаг уменьшения масштаба
    double scale_dec_step_coeff = 1.0 / sqrt(2.0);

    /// Текущее смещение координат
    QPoint map_shift;

    /// Положение курсора в момент последнего нажатия ЛКМ
    QPoint mouse_pos_LBpressed;

    /// Смещение координат до движения курсора с зажатой ЛКМ
    QPoint prev_map_shift;

    /// Флаг отображения имен траекторий
    bool show_traj_names = false;

    /// Перемещение вслед за игроком
    bool follow_player = true;

    /// Выбранный игрок
    int follow_player_idx = 0;

    /// Длина отрисовки выбранной траектории стрелки, м
    double switch_length = 35.0;

    /// Радиус у схематичного отображения сигналов светофоров, м
    double signal_radius = 2.0;

    /// Смещение схематичного светофора вправо от оси пути, м
    double signal_offset = 2.5;

    void paintEvent(QPaintEvent *event);

    void drawTrajectory(Trajectory* traj, QPointF& cursor_pos, double& distance2);

    void drawTrains(simulator_update_pos_t *train_data);

    void drawTrainNames(simulator_update_pos_t *train_data);

    void drawVehicle(simulator_vehicle_pos_update_t &vehicle, double &vehicle_half_length, QColor color);

    void drawConnector(Connector* conn, QPointF& cursor_pos, double& distance2);

    void drawStations(topology_stations_list_t *stations);

    void drawSignals(signals_data_t *signals_data);

    void drawSignal(Signal *signal, std::vector<QColor> lens_colors);

    QPoint coord_transform(dvec3 point);

    double distance2_pos_to_point(const QPointF& pos, const QPointF& point);

    double distance_pos_to_point(const QPointF& pos, const QPointF& point);

    double distance2_pos_to_line_segment(const QPointF& pos, const QPointF& pointA, const QPointF& pointB);

    double distance_pos_to_line_segment(const QPointF& pos, const QPointF& pointA, const QPointF& pointB);

    void wheelEvent(QWheelEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void mousePressEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // MAP_WIDGET_H
