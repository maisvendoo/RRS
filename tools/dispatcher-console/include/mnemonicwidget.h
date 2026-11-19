#ifndef MNEMONICWIDGET_H
#define MNEMONICWIDGET_H

#include <QWidget>
#include <QVector>
#include <QMap>
#include <QPointF>
#include <QColor>
#include <QString>
#include <QByteArray>
#include <cstdint>

#include <stationmodel.h>

struct hit_result_t
{
    enum Type { NONE, SIGNAL, CONNECTOR, STATION, TRAIN };
    Type type = NONE;
    int index = -1;
    QString name;
    QPointF screen_pos;
};

struct signal_hit_t
{
    QPointF screen_pos;
    int signal_type = 0;
    int signal_index = 0;
};

struct signal_view_t
{
    QString conn_name;
    int8_t signal_dir = 0;
    QString letter;
    QString signal_model;
    bool lens_red = false, lens_yellow = false, lens_green = false;
    bool lens_white = false, lens_blue = false, lens_bot_yellow = false;
    double pos_x = 0.0, pos_y = 0.0;
    QColor lensColor() const;
};

struct switch_view_t
{
    QString name;
    int8_t state_fwd = 0, state_bwd = 0;
    double pos_x = 0.0, pos_y = 0.0;
};

struct traj_busy_view_t
{
    QString name;
    bool is_busy = false, in_route = false;
};

struct vehicle_view_t
{
    int vehicle_id = -1;
    double pos_x = 0.0, pos_y = 0.0;
    double orth_x = 0.0, orth_y = 0.0;
    double length = 0.0;
};

struct train_view_t
{
    int first_vehicle = -1, last_vehicle = -1;
    QString name;
};

struct sim_time_t
{
    int h = 0, m = 0, s = 0;
    QString getString() const;
};

struct train_pos_update_t
{
    sim_time_t sim_time;
    int speed_factor = 0;
    QVector<vehicle_view_t> vehicles;
};

struct dispatcher_data_t
{
    QVector<signal_view_t> line_signals, enter_signals, route_signals, exit_signals, shunt_signals;
    QMap<QString, switch_view_t> switches;
    QMap<QString, traj_busy_view_t> trajectory_states;
    QVector<train_view_t> trains;
    train_pos_update_t pos_update;

    static bool deserializeSignals(QByteArray &data, dispatcher_data_t &out);
    static bool deserializeSwitch(QByteArray &data, switch_view_t &out);
    static bool deserializeTrajBusy(QByteArray &data, traj_busy_view_t &out);
    static bool deserializeTrains(QByteArray &data, QVector<train_view_t> &out);
    static bool deserializePos(QByteArray &data, train_pos_update_t &out);
};

class MnemonicWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MnemonicWidget(QWidget *parent = nullptr);

    void setStationModel(const StationModel *model);
    void setDispatcherData(const dispatcher_data_t *data);
    void setScale(double s);
    double scale() const;
    void setOffset(double o);
    double offset() const;

    // Поиск объектов под курсором
    hit_result_t hitTest(const QPoint &screen_pos) const;

    // Получить данные о сигнале по hit-результату
    const signal_view_t *signalByHit(const hit_result_t &h) const;

signals:
    void signalRightClicked(const signal_view_t *sig);
    void connectorRightClicked(const QString &name);
    void trainRightClicked(int vehicle_id);
    void stationLeftClicked(const QString &name);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    const StationModel *m_model = nullptr;
    const dispatcher_data_t *m_data = nullptr;
    double m_scale = 1.0, m_offset = 0.0;
    bool m_dragging = false;
    QPoint m_last_mouse_pos;
    int m_hovered_station = -1;

    static constexpr double STATION_W = 120.0, STATION_H = 60.0;
    static constexpr double STATION_GAP = 300.0, VCENTER = 300.0;
    static constexpr double HIT_RADIUS = 12.0;

    QPointF stationCenter(int idx) const;
    void drawStation(QPainter &p, const station_info_t &s, QPointF c);
    void drawBlockSection(QPainter &p, QPointF from, QPointF to, bool busy, bool in_route);
    void drawSignalAt(QPainter &p, const signal_view_t &sig, QPointF pos,
                      int sig_type, int sig_idx, QVector<signal_hit_t> &hits);
    void drawConnectorAt(QPainter &p, const switch_view_t &sw, QPointF pos);
    void drawTrain(QPainter &p, const vehicle_view_t &v, const QString &name);

    QVector<signal_hit_t> m_signal_hits;
    QVector<QPair<QPointF, QString>> m_connector_hits;
    QVector<QPair<QPointF, int>> m_train_hits;
    QVector<QPointF> m_station_hits;
};

#endif