#ifndef     MAP_WIDGET_H
#define     MAP_WIDGET_H

#include    <QMap>
#include    <topology-types.h>
#include    <trajectory.h>
#include    <simulator-update-struct.h>
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

    sw_list_t *conn_list = nullptr;

    Trajectory* route_begin_trajectory = nullptr;

    Trajectory* nearest_trajectory = nullptr;

    Switch* nearest_switch = nullptr;

    std::int8_t nearest_switch_dir = 0;

    /// Флаг отображения имён траекторий
    bool show_traj_names = false;

    /// Флаг отображения имён коннекторов
    bool show_conn_names = false;

    simulator_update_players_t *players_data = nullptr;

    simulator_update_pos_t *train_data = nullptr;

    std::vector<double> *vehicles_half_length = nullptr;

    topology_stations_list_t *stations = nullptr;

    signals_data_t *signals_data = nullptr;

    QMap<QString, SignalLabel *> signal_labels_fwd;

    QMap<QString, SignalLabel *> signal_labels_bwd;

    QMap<QString, QLabel *> traj_labels;

    QMap<QString, QLabel *> switch_labels;

    std::vector<TrainLabel *> train_labels;

    struct switch_menu_t {
        QMenu* menu = nullptr;
        QAction* action = nullptr;
        Switch* conn = nullptr;
        std::int8_t switch_dir = 0;
    } switch_menu;

    void resize(int width, int height);

    void setSwitchLength(double value);

    void setSignalRadius(double value);

    void setSignalOffset(double value);

    void calcCwitchCoords();

    double getScale() const
    {
        return scale;
    }

    QPoint getShift() const
    {
        return map_shift;
    }

signals:

    void sigOpenSwitchMenu(Switch* nearest_conn, std::int8_t nearest_switch_dir);

    void sigOpenTrajectoryMenu(Trajectory* nearest_traj);

    void sigSelectNearestTrajectory(Trajectory* nearest_traj);

public slots:

    void slotStationAtCenter(int idx);

    void slotPlayerAtCenter(int idx);

    void resetSwitchMenu();

private:

    /// Траектория
    static constexpr QColor color_traj_free = QColor(0, 0, 0);
    /// Траектория занята подвижным составом
    static constexpr QColor color_traj_busy = QColor(255, 0, 0);
    /// Траектория включена в маршрут ДЦ
    static constexpr QColor color_traj_route = QColor(255, 255, 0);

    /// Коннектор
    static constexpr QColor color_connector = QColor(96, 96, 96);
    /// Стрелка
    static constexpr QColor color_switch_free = QColor(0, 0, 128);
    /// Стрелка занята подвижным составом
    static constexpr QColor color_switch_busy = QColor(96, 96, 96);
    /// Стрелка включена в маршрут ДЦ
    static constexpr QColor color_switch_route = QColor(255, 192, 96);
    /// Другое направление стрелки
    static constexpr QColor color_switch_other = color_traj_free;
    /// Другое направление стрелки при выборе пункта меню с переключением
    static constexpr QColor color_switch_other_selected = QColor(0, 128, 255);

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

    struct switch_coord_t {
        dvec3 center;
        dvec3 orth;
        dvec3 trav;
    };
    QMap<QString, switch_coord_t> switch_coords;

    void paintEvent(QPaintEvent *event);

    void drawTrajectory(Trajectory* traj, QPainter& painter,
                        QPointF& cursor_pos, double& distance2);

    void drawTrains(simulator_update_pos_t *train_data, QPainter& painter);

    void drawTrainNames(simulator_update_pos_t *train_data, QPainter& painter);

    void drawVehicle(simulator_vehicle_pos_update_t &vehicle, double &vehicle_half_length,
                     QPainter& painter, QColor color);

    void drawConnector(Switch* conn, QPainter& painter,
                       QPointF& cursor_pos, double& distance2, std::int8_t& dir);

    void drawSwitchTraj(Trajectory* traj, bool draw_to_fwd, QPainter& painter,
                        QPointF& cursor_pos, double& distance2, std::int8_t& dir);

    void drawStations(topology_stations_list_t *stations, QPainter& painter);

    void drawSignals(signals_data_t *signals_data, QPainter& painter);

    void drawSignal(Signal* signal, QPainter& painter, std::vector<QColor> lens_colors);

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
