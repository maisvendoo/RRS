#ifndef MNEMONICWIDGET_H
#define MNEMONICWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPointF>

#include <stationmodel.h>

class MnemonicWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MnemonicWidget(QWidget *parent = nullptr);

    void setStationModel(const StationModel *model);

    void setScale(double scale);
    double scale() const;

    void setOffset(double offset);
    double offset() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    const StationModel *m_model = nullptr;

    double m_scale = 1.0;
    double m_offset = 0.0;

    bool m_dragging = false;
    QPoint m_last_mouse_pos;

    static constexpr double STATION_WIDTH = 120.0;
    static constexpr double STATION_HEIGHT = 60.0;
    static constexpr double BLOCK_SECTION_LENGTH = 200.0;
    static constexpr double VERTICAL_CENTER = 300.0;
    static constexpr double STATION_GAP = 300.0;

    void drawStation(QPainter &painter, const station_info_t &station, QPointF center);
    void drawBlockSection(QPainter &painter, QPointF from, QPointF to);
};

#endif