#include    <mnemonicwidget.h>
#include    <QPainter>
#include    <QWheelEvent>
#include    <QMouseEvent>
#include    <QFontMetrics>
#include    <QDataStream>
#include    <QIODevice>
#include    <QtMath>
#include    <QLineF>

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
    auto rd = [&](QVector<signal_view_t> &v) -> bool {
        quint32 c; st >> c; v.resize(c);
        for (quint32 i = 0; i < c; ++i) { signal_view_t s; if (!readSignal_t(st, s)) return false; v[i] = s; }
        return true;
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
        auto d5 = [](uint8_t *b) -> double {
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
    setMouseTracking(true); setMinimumSize(600, 400);
}

void MnemonicWidget::setStationModel(const StationModel *m) { m_model = m; update(); }
void MnemonicWidget::setDispatcherData(const dispatcher_data_t *d) { m_data = d; update(); }
void MnemonicWidget::setScale(double s) { m_scale = qBound(0.2, s, 5.0); update(); }
double MnemonicWidget::scale() const { return m_scale; }
void MnemonicWidget::setOffset(double o) { m_offset = o; update(); }
double MnemonicWidget::offset() const { return m_offset; }

QPointF MnemonicWidget::stationCenter(int idx) const
{
    if (!m_model) return {0, VCENTER};
    double t = m_model->stations().size() * STATION_GAP * m_scale;
    double sx = (width() - t) / 2.0 + m_offset;
    return {sx + idx * STATION_GAP * m_scale + STATION_W * m_scale / 2.0, VCENTER * m_scale};
}

hit_result_t MnemonicWidget::hitTest(const QPoint &sp) const
{
    hit_result_t res;
    double best_dist = HIT_RADIUS * m_scale;
    // Сигналы
    for (const auto &sh : m_signal_hits)
    {
        double d = QLineF(QPointF(sp), sh.screen_pos).length();
        if (d < best_dist) { best_dist = d; res.type = hit_result_t::SIGNAL; res.index = sh.signal_index; res.screen_pos = sh.screen_pos; }
    }
    // Коннекторы (стрелки)
    for (const auto &cp : m_connector_hits)
    {
        double d = QLineF(QPointF(sp), cp.first).length();
        if (d < best_dist) { best_dist = d; res.type = hit_result_t::CONNECTOR; res.name = cp.second; res.screen_pos = cp.first; }
    }
    // Поезда
    for (const auto &tp : m_train_hits)
    {
        double d = QLineF(QPointF(sp), tp.first).length();
        if (d < best_dist) { best_dist = d; res.type = hit_result_t::TRAIN; res.index = tp.second; res.screen_pos = tp.first; }
    }
    // Станции
    for (int i = 0; i < m_station_hits.size(); ++i)
    {
        double d = QLineF(QPointF(sp), m_station_hits[i]).length();
        if (d < best_dist) { best_dist = d; res.type = hit_result_t::STATION; res.index = i; res.screen_pos = m_station_hits[i]; }
    }
    return res;
}

const signal_view_t *MnemonicWidget::signalByHit(const hit_result_t &h) const
{
    if (h.type != hit_result_t::SIGNAL || !m_data) return nullptr;
    int idx = h.index;
    if (idx < m_data->line_signals.size()) return &m_data->line_signals[idx];
    idx -= m_data->line_signals.size();
    if (idx < m_data->enter_signals.size()) return &m_data->enter_signals[idx];
    idx -= m_data->enter_signals.size();
    if (idx < m_data->route_signals.size()) return &m_data->route_signals[idx];
    idx -= m_data->route_signals.size();
    if (idx < m_data->exit_signals.size()) return &m_data->exit_signals[idx];
    idx -= m_data->exit_signals.size();
    if (idx < m_data->shunt_signals.size()) return &m_data->shunt_signals[idx];
    return nullptr;
}

void MnemonicWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(30, 30, 35));

    if (!m_model || m_model->isEmpty())
    {
        p.setPen(Qt::white); p.drawText(rect(), Qt::AlignCenter, tr("No data"));
        return;
    }

    m_signal_hits.clear();
    m_connector_hits.clear();
    m_train_hits.clear();
    m_station_hits.clear();

    const auto &ss = m_model->stations();

    // Перегоны
    for (int i = 0; i < ss.size() - 1; ++i)
    {
        QPointF a = stationCenter(i), b = stationCenter(i + 1);
        a.setX(a.x() + STATION_W * m_scale / 2.0);
        b.setX(b.x() - STATION_W * m_scale / 2.0);

        // Проверяем занятость перегона
        bool busy = false, in_route = false;
        if (m_data)
        {
            for (auto it = m_data->trajectory_states.constBegin(); it != m_data->trajectory_states.constEnd(); ++it)
            {
                if (it.value().is_busy) busy = true;
                if (it.value().in_route) in_route = true;
            }
        }
        drawBlockSection(p, a, b, busy, in_route);
    }

    // Станции, сигналы и стрелки у станций
    for (int i = 0; i < ss.size(); ++i)
    {
        QPointF c = stationCenter(i);
        m_station_hits.append(c);
        drawStation(p, ss[i], c);

        if (!m_data) continue;

        // Станционные сигналы
        auto drawSigVec = [&](const QVector<signal_view_t> &vec, int base_type, bool above) {
            double yo = above ? -45 * m_scale : 45 * m_scale;
            for (int j = 0; j < vec.size(); ++j)
            {
                double x = c.x() + (j - (double)(vec.size() - 1) / 2.0) * 20 * m_scale;
                QPointF sp(x, c.y() + yo);
                drawSignalAt(p, vec[j], sp, base_type, j, m_signal_hits);
            }
        };
        drawSigVec(m_data->enter_signals, 1, true);
        drawSigVec(m_data->route_signals, 2, true);
        drawSigVec(m_data->exit_signals, 3, false);
        drawSigVec(m_data->shunt_signals, 4, false);
    }

    if (!m_data) return;

    // Линейные сигналы
    for (int i = 0; i < m_data->line_signals.size(); ++i)
    {
        double ratio = (double)(i + 1) / (m_data->line_signals.size() + 1);
        double x = stationCenter(0).x() + (stationCenter(ss.size() - 1).x() - stationCenter(0).x()) * ratio;
        QPointF sp(x, VCENTER * m_scale - 40 * m_scale);
        drawSignalAt(p, m_data->line_signals[i], sp, 0, i, m_signal_hits);
    }

    // Стрелки (коннекторы) — отображаем у станций
    int sw_idx = 0;
    for (auto it = m_data->switches.constBegin(); it != m_data->switches.constEnd(); ++it, ++sw_idx)
    {
        int station_idx = sw_idx % qMax(1, ss.size());
        QPointF c = stationCenter(station_idx);
        QPointF sp(c.x(), c.y() + 70 * m_scale + (sw_idx / ss.size()) * 20 * m_scale);
        m_connector_hits.append({sp, it.key()});
        drawConnectorAt(p, it.value(), sp);
    }

    // Поезда
    for (const auto &tv : m_data->trains)
    {
        QString name = tv.name.isEmpty() ? tr("Train") : tv.name;
        for (int vi = tv.first_vehicle; vi <= tv.last_vehicle && vi < m_data->pos_update.vehicles.size(); ++vi)
        {
            const auto &v = m_data->pos_update.vehicles[vi];
            // Находим ближайшую станцию
            int ni = 0; double md = 1e9;
            for (int i = 0; i < ss.size(); ++i)
            {
                double d = qAbs(ss[i].pos_y - v.pos_y) + qAbs(ss[i].pos_x - v.pos_x);
                if (d < md) { md = d; ni = i; }
            }
            QPointF c = stationCenter(ni);
            double px = c.x() + (v.pos_x - ss[ni].pos_x) * 0.5 * m_scale;
            QPointF tp(px, VCENTER * m_scale);
            m_train_hits.append({tp, vi});
            drawTrain(p, v, tv.name);
        }
    }

    // Отображение времени симуляции
    QString t = m_data->pos_update.sim_time.getString();
    if (!t.isEmpty())
    {
        p.setPen(QColor(180, 180, 180));
        QFont f("Consolas", 22, QFont::Bold);
        p.setFont(f);
        p.drawText(rect().adjusted(-10, 10, 0, 0), Qt::AlignRight | Qt::AlignTop, t);
    }

    // Индикатор зума
    QString z = tr("Zoom: %1%").arg((int)(m_scale * 100));
    p.setPen(QColor(120, 120, 120));
    p.setFont(QFont("Arial", 10));
    p.drawText(rect().adjusted(10, -10, 0, 0), Qt::AlignLeft | Qt::AlignBottom, z);
}

void MnemonicWidget::drawStation(QPainter &p, const station_info_t &s, QPointF c)
{
    double w = STATION_W * m_scale, h = STATION_H * m_scale;
    QRectF r(c.x() - w / 2.0, c.y() - h / 2.0, w, h);

    p.setPen(QPen(QColor(100, 160, 255), 2.5));
    p.setBrush(QColor(45, 75, 150));
    p.drawRect(r);

    p.setPen(Qt::white);
    QFont f("Arial", qMax(8, (int)(11 * m_scale)));
    p.setFont(f);
    p.drawText(r, Qt::AlignCenter, s.name);

    // Подсветка при наведении
    if (m_hovered_station >= 0 && m_station_hits.size() > m_hovered_station)
    {
        QPointF hc = m_station_hits[m_hovered_station];
        if (QLineF(c, hc).length() < 1.0)
        {
            p.setPen(QPen(QColor(255, 255, 100), 3));
            p.drawRect(r);
        }
    }
}

void MnemonicWidget::drawBlockSection(QPainter &p, QPointF from, QPointF to, bool busy, bool in_route)
{
    QColor color;
    if (busy) color = QColor(200, 60, 60);
    else if (in_route) color = QColor(60, 180, 60);
    else color = QColor(80, 120, 80);

    int w = qMax(2, (int)(3 * m_scale));
    p.setPen(QPen(color, w));
    p.drawLine(from, to);
}

void MnemonicWidget::drawSignalAt(QPainter &p, const signal_view_t &sig, QPointF pos,
                                   int sig_type, int sig_idx, QVector<signal_hit_t> &hits)
{
    int r = qMax(4, (int)(6 * m_scale));
    p.setBrush(sig.lensColor());
    p.setPen(QPen(Qt::white, 1.5));
    p.drawEllipse(pos, r, r);

    if (!sig.letter.isEmpty())
    {
        p.setPen(Qt::white);
        p.setFont(QFont("Arial", qMax(7, (int)(9 * m_scale))));
        p.drawText(QPointF(pos.x() + r + 2, pos.y() + 4), sig.letter);
    }

    hits.append({pos, sig_type, sig_idx});
}

void MnemonicWidget::drawConnectorAt(QPainter &p, const switch_view_t &sw, QPointF pos)
{
    // Определяем цвет по состоянию
    QColor col;
    if (sw.state_fwd > 0 || sw.state_bwd > 0) col = QColor(100, 200, 100);
    else if (sw.state_fwd < 0 || sw.state_bwd < 0) col = QColor(200, 200, 100);
    else col = QColor(150, 150, 150);

    int r = qMax(4, (int)(5 * m_scale));
    p.setBrush(col);
    p.setPen(QPen(Qt::white, 1));
    p.drawEllipse(pos, r, r);
    p.drawText(QPointF(pos.x() + r + 2, pos.y() + 3), sw.name.right(6));
}

void MnemonicWidget::drawTrain(QPainter &p, const vehicle_view_t &v, const QString &name)
{
    if (!m_model || m_model->stations().empty()) return;
    const auto &ss = m_model->stations();
    int ni = 0; double md = 1e9;
    for (int i = 0; i < ss.size(); ++i)
    {
        double d = qAbs(ss[i].pos_y - v.pos_y) + qAbs(ss[i].pos_x - v.pos_x);
        if (d < md) { md = d; ni = i; }
    }
    QPointF cs = stationCenter(ni);
    double x = cs.x() + (v.pos_x - ss[ni].pos_x) * 0.5 * m_scale;
    QPointF tp(x, VCENTER * m_scale);

    int r = qMax(5, (int)(7 * m_scale));
    p.setBrush(QColor(220, 220, 50));
    p.setPen(QPen(Qt::white, 1.5));
    p.drawEllipse(tp, r, r);

    if (!name.isEmpty())
    {
        p.setPen(Qt::white);
        p.setFont(QFont("Arial", qMax(7, (int)(9 * m_scale))));
        p.drawText(QPointF(tp.x() + r + 3, tp.y() + 4), name);
    }
}

void MnemonicWidget::wheelEvent(QWheelEvent *e)
{
    setScale(m_scale * ((e->angleDelta().y() > 0) ? 1.15 : 1.0 / 1.15));
    e->accept();
}

void MnemonicWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        // Проверка клика по станции
        hit_result_t h = hitTest(e->pos());
        if (h.type == hit_result_t::STATION && m_model && h.index < m_model->stations().size())
        {
            emit stationLeftClicked(m_model->stations()[h.index].name);
            return;
        }
        m_dragging = true;
        m_last_mouse_pos = e->pos();
    }
    if (e->button() == Qt::RightButton)
    {
        hit_result_t h = hitTest(e->pos());
        if (h.type == hit_result_t::SIGNAL)
        {
            const signal_view_t *sig = signalByHit(h);
            if (sig) emit signalRightClicked(sig);
        }
        else if (h.type == hit_result_t::CONNECTOR)
        {
            emit connectorRightClicked(h.name);
        }
        else if (h.type == hit_result_t::TRAIN)
        {
            emit trainRightClicked(h.index);
        }
    }
}

void MnemonicWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging)
    {
        m_offset += e->pos().x() - m_last_mouse_pos.x();
        m_last_mouse_pos = e->pos();
        update();
    }
    // Обновляем подсветку при наведении
    hit_result_t h = hitTest(e->pos());
    int old_hover = m_hovered_station;
    m_hovered_station = (h.type == hit_result_t::STATION) ? h.index : -1;
    if (old_hover != m_hovered_station) update();
}

void MnemonicWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) m_dragging = false;
}