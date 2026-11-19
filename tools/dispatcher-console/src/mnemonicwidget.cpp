#include    <mnemonicwidget.h>

#include    <QPainter>
#include    <QWheelEvent>
#include    <QMouseEvent>
#include    <QFontMetrics>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MnemonicWidget::MnemonicWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumSize(400, 400);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MnemonicWidget::setStationModel(const StationModel *model)
{
    m_model = model;
    update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MnemonicWidget::setScale(double s)
{
    m_scale = qBound(0.2, s, 5.0);
    update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double MnemonicWidget::scale() const
{
    return m_scale;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MnemonicWidget::setOffset(double o)
{
    m_offset = o;
    update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double MnemonicWidget::offset() const
{
    return m_offset;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MnemonicWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(40, 40, 40));

    if (!m_model || m_model->isEmpty())
    {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, tr("No station data"));
        return;
    }

    const auto &stations = m_model->stations();
    if (stations.isEmpty())
        return;

    double total_width = stations.size() * STATION_GAP * m_scale;
    double start_x = (width() - total_width) / 2.0 + m_offset;

    // Рисуем перегоны между станциями
    for (int i = 0; i < stations.size() - 1; ++i)
    {
        QPointF from(start_x + i * STATION_GAP * m_scale + STATION_WIDTH * m_scale / 2.0,
                     VERTICAL_CENTER * m_scale);
        QPointF to(start_x + (i + 1) * STATION_GAP * m_scale + STATION_WIDTH * m_scale / 2.0,
                   VERTICAL_CENTER * m_scale);
        drawBlockSection(painter, from, to);
    }

    // Рисуем станции
    for (int i = 0; i < stations.size(); ++i)
    {
        QPointF center(start_x + i * STATION_GAP * m_scale + STATION_WIDTH * m_scale / 2.0,
                       VERTICAL_CENTER * m_scale);
        drawStation(painter, stations[i], center);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MnemonicWidget::drawStation(QPainter &painter, const station_info_t &station, QPointF center)
{
    double w = STATION_WIDTH * m_scale;
    double h = STATION_HEIGHT * m_scale;
    QRectF rect(center.x() - w / 2.0, center.y() - h / 2.0, w, h);

    // Корпус станции
    painter.setPen(QPen(QColor(100, 150, 255), 2));
    painter.setBrush(QColor(50, 80, 160));
    painter.drawRect(rect);

    // Название станции
    painter.setPen(Qt::white);
    QFont font("Arial", qMax(8, static_cast<int>(10 * m_scale)));
    painter.setFont(font);
    painter.drawText(rect, Qt::AlignCenter, station.name);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MnemonicWidget::drawBlockSection(QPainter &painter, QPointF from, QPointF to)
{
    painter.setPen(QPen(QColor(80, 120, 80), 3 * m_scale));
    painter.drawLine(from, to);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MnemonicWidget::wheelEvent(QWheelEvent *event)
{
    double factor = (event->angleDelta().y() > 0) ? 1.15 : 1.0 / 1.15;
    setScale(m_scale * factor);
    event->accept();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MnemonicWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragging = true;
        m_last_mouse_pos = event->pos();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MnemonicWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging)
    {
        double dx = event->pos().x() - m_last_mouse_pos.x();
        m_offset += dx;
        m_last_mouse_pos = event->pos();
        update();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MnemonicWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_dragging = false;
}