#include    <mnemonicwidget.h>
#include    <QPainter>
#include    <QWheelEvent>
#include    <QMouseEvent>
#include    <QFontMetrics>
#include    <QDataStream>
#include    <QIODevice>
#include    <QtMath>

QColor signal_view_t::lensColor() const
{
    if (lens_red) return Qt::red;
    if (lens_green) return Qt::green;
    if (lens_yellow) return QColor(255, 255, 0);
    if (lens_white) return Qt::white;
    if (lens_blue) return Qt::blue;
    if (lens_bot_yellow) return QColor(255, 200, 0);
    return QColor(60, 60, 60);
}

QString sim_time_t::getString() const
{
    return QString("%1:%2:%3").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}

static bool readSignal_t(QDataStream &st, signal_view_t &sig)
{
    st >> sig.conn_name >> sig.signal_dir >> sig.letter >> sig.signal_model;
    uint8_t lens[8];
    for (int i = 0; i < 8; ++i) st >> lens[i];
    sig.lens_red = lens[0]; sig.lens_yellow = lens[1]; sig.lens_green = lens[2];
    sig.lens_bot_yellow = lens[3]; sig.lens_white = lens[4]; sig.lens_blue = lens[5];
    double px, py, pz, ox, oy, oz, rx, ry, rz, ux, uy, uz;
    st >> px >> py >> pz >> ox >> oy >> oz >> rx >> ry >> rz >> ux >> uy >> uz;
    sig.pos_x = px; sig.pos_y = py;
    return st.status() == QDataStream::Ok;
}

bool dispatcher_data_t::deserializeSignals(QByteArray &data, dispatcher_data_t &out)
{
    QDataStream st(&data, QIODevice::ReadOnly);
    st.setByteOrder(QDataStream::BigEndian); st.setVersion(QDataStream::Qt_4_0);
    auto rd = [&](QVector<signal_view_t> &v) {
        quint32 c; st >> c; v.resize(c);
        for (quint32 i = 0; i < c; ++i) { signal_view_t s; readSignal_t(st, s); v[i] = s; }
        return st.status() == QDataStream::Ok;
    };
    return rd(out.line_signals) && rd(out.enter_signals) && rd(out.route_signals)
           && rd(out.exit_signals) && rd(out.shunt_signals);
}

bool dispatcher_data_t::deserializeSwitch(QByteArray &data, switch_view_t &out)
{
    QDataStream st(&data, QIODevice::ReadOnly);
    st.setByteOrder(QDataStream::BigEndian); st.setVersion(QDataStream::Qt_4_0);
    st >> out.name >> out.state_fwd >> out.state_bwd;
    return st.status() == QDataStream::Ok;
}

bool dispatcher_data_t::deserializeTrajBusy(QByteArray &data, traj_busy_view_t &out)
{
    QDataStream st(&data, QIODevice::ReadOnly);
    st.setByteOrder(QDataStream::BigEndian); st.setVersion(QDataStream::Qt_4_0);
    st >> out.name >> out.is_busy >> out.in_route;
    return st.status() == QDataStream::Ok;
}

bool dispatcher_data_t::deserializeTrains(QByteArray &data, QVector<train_view_t> &out)
{
    QDataStream st(&data, QIODevice::ReadOnly);
    st.setByteOrder(QDataStream::BigEndian); st.setVersion(QDataStream::Qt_4_0);
    quint32 c; st >> c; out.resize(c);
    for (quint32 i = 0; i < c; ++i) st >> out[i].first_vehicle >> out[i].last_vehicle >> out[i].name;
    return st.status() == QDataStream::Ok;
}

bool dispatcher_data_t::deserializePos(QByteArray &data, train_pos_update_t &out)
{
    QDataStream st(&data, QIODevice::ReadOnly);
    st.setByteOrder(QDataStream::BigEndian); st.setVersion(QDataStream::Qt_4_0);
    st >> out.speed_factor >> out.sim_time.h >> out.sim_time.m >> out.sim_time.s;
    quint32 c; st >> c; out.vehicles.resize(c);
    for (quint32 i = 0; i < c; ++i)
    {
        uint8_t px[5], py[5], pz[5];
        st.readRawData((char*)px, 5); st.readRawData((char*)py, 5); st.readRawData((char*)pz, 5);
        auto d5 = [](uint8_t *b) {
            int64_t v = 0; for (int j = 0; j < 5; ++j) v = (v << 8) | b[j];
            if (v & (INT64_C(1) << 39)) v |= ~((INT64_C(1) << 40) - 1);
            return (double)v * 0.0002;
        };
        out.vehicles[i].pos_x = d5(px); out.vehicles[i].pos_y = d5(py);
        uint8_t of; uint16_t oc[2]; uint8_t uf; uint16_t uc[2];
        st >> of >> oc[0] >> oc[1] >> uf >> uc[0] >> uc[1];
        out.vehicles[i].orth_x = (double)(int16_t)oc[0] / 32767.0;
        out.vehicles[i].orth_y = (double)(int16_t)oc[1] / 32767.0;
        out.vehicles[i].vehicle_id = (int)i;
    }
    return st.status() == QDataStream::Ok;
}

MnemonicWidget::MnemonicWidget(QWidget *p) : QWidget(p)
{
    setMouseTracking(true); setMinimumSize(400, 400);
}

void MnemonicWidget::setStationModel(const StationModel *m) { m_model = m; update(); }
void MnemonicWidget::setDispatcherData(const dispatcher_data_t *d) { m_data = d; update(); }
void MnemonicWidget::setScale(double s) { m_scale = qBound(0.2, s, 5.0); update(); }
double MnemonicWidget::scale() const { return m_scale; }
void MnemonicWidget::setOffset(double o) { m_offset = o; update(); }
double MnemonicWidget::offset() const { return m_offset; }

QPointF MnemonicWidget::stationCenter(int idx) const
{
    if (!m_model) return {};
    double t = m_model->stations().size() * STATION_GAP * m_scale;
    double sx = (width() - t) / 2.0 + m_offset;
    return {sx + idx * STATION_GAP * m_scale + STATION_W * m_scale / 2.0, VCENTER * m_scale};
}

void MnemonicWidget::paintEvent(QPaintEvent *)
{
    QPainter pt(this);
    pt.setRenderHint(QPainter::Antialiasing);
    pt.fillRect(rect(), QColor(35, 35, 40));
    if (!m_model || m_model->isEmpty())
    {
        pt.setPen(Qt::white); pt.drawText(rect(), Qt::AlignCenter, tr("No data"));
        return;
    }
    const auto &ss = m_model->stations();
    for (int i = 0; i < ss.size() - 1; ++i)
    {
        QPointF a = stationCenter(i), b = stationCenter(i + 1);
        a.setX(a.x() + STATION_W * m_scale / 2.0);
        b.setX(b.x() - STATION_W * m_scale / 2.0);
        drawBlock(pt, a, b);
    }
    for (int i = 0; i < ss.size(); ++i)
    {
        QPointF c = stationCenter(i);
        drawStation(pt, ss[i], c);
        if (m_data)
        {
            drawSignals(pt, m_data->enter_signals, c, true);
            drawSignals(pt, m_data->exit_signals, c, false);
            drawSignals(pt, m_data->route_signals, c, true);
            drawSignals(pt, m_data->shunt_signals, c, false);
        }
    }
    if (m_data)
    {
        for (int i = 0; i < m_data->line_signals.size(); ++i)
        {
            QPointF a = stationCenter(ss.size() - 1);
            double r = (double)(i + 1) / (m_data->line_signals.size() + 1);
            double x = stationCenter(0).x() + (a.x() - stationCenter(0).x()) * r;
            const auto &s = m_data->line_signals[i];
            pt.setBrush(s.lensColor()); pt.setPen(Qt::white);
            int rr = qMax(4, (int)(6 * m_scale));
            pt.drawEllipse(QPointF(x, VCENTER * m_scale - 30 * m_scale), rr, rr);
            pt.drawText(QPointF(x + rr + 3, VCENTER * m_scale - 30 * m_scale + 4), s.letter);
        }
        for (const auto &v : m_data->pos_update.vehicles)
            drawTrain(pt, v);
    }
}

void MnemonicWidget::drawStation(QPainter &p, const station_info_t &s, QPointF c)
{
    QRectF r(c.x() - STATION_W * m_scale / 2.0, c.y() - STATION_H * m_scale / 2.0,
             STATION_W * m_scale, STATION_H * m_scale);
    p.setPen(QPen(QColor(100, 150, 255), 2)); p.setBrush(QColor(50, 80, 160));
    p.drawRect(r); p.setPen(Qt::white);
    p.setFont(QFont("Arial", qMax(8, (int)(10 * m_scale))));
    p.drawText(r, Qt::AlignCenter, s.name);
}

void MnemonicWidget::drawBlock(QPainter &p, QPointF from, QPointF to)
{
    bool busy = false;
    if (m_data) for (auto it = m_data->trajectory_states.constBegin(); it != m_data->trajectory_states.constEnd(); ++it)
        if (it.value().is_busy) { busy = true; break; }
    p.setPen(QPen(busy ? QColor(200, 80, 80) : QColor(80, 120, 80), qMax(2, (int)(3 * m_scale))));
    p.drawLine(from, to);
}

void MnemonicWidget::drawSignals(QPainter &p, const QVector<signal_view_t> &sv, QPointF sp, bool ab)
{
    double yo = ab ? -40 * m_scale : 40 * m_scale;
    for (int i = 0; i < sv.size(); ++i)
    {
        const auto &s = sv[i];
        double x = sp.x() + (i - (double)(sv.size() - 1) / 2.0) * 15 * m_scale;
        QPointF sp2(x, sp.y() + yo);
        p.setBrush(s.lensColor()); p.setPen(Qt::white);
        int r = qMax(3, (int)(5 * m_scale));
        p.drawEllipse(sp2, r, r);
        p.drawText(QPointF(sp2.x() + r + 2, sp2.y() + 3), s.letter);
    }
}

void MnemonicWidget::drawTrain(QPainter &p, const vehicle_view_t &v)
{
    if (!m_model || m_model->stations().empty()) return;
    const auto &ss = m_model->stations();
    int ni = 0; double md = 1e9;
    for (int i = 0; i < ss.size(); ++i)
    {
        double d = qAbs(ss[i].pos_y - v.pos_y) + qAbs(ss[i].pos_x - v.pos_x);
        if (d < md) { md = d; ni = i; }
    }
    QPointF c = stationCenter(ni);
    double x = c.x() + (v.pos_x - ss[ni].pos_x) * 0.5 * m_scale;
    p.setBrush(QColor(200, 200, 50)); p.setPen(Qt::white);
    int r = qMax(4, (int)(6 * m_scale));
    p.drawEllipse(QPointF(x, VCENTER * m_scale), r, r);
}

void MnemonicWidget::wheelEvent(QWheelEvent *e)
{
    setScale(m_scale * ((e->angleDelta().y() > 0) ? 1.15 : 1.0 / 1.15));
    e->accept();
}

void MnemonicWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) { m_dragging = true; m_last_mouse_pos = e->pos(); }
}

void MnemonicWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging) { m_offset += e->pos().x() - m_last_mouse_pos.x(); m_last_mouse_pos = e->pos(); update(); }
}

void MnemonicWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) m_dragging = false;
}