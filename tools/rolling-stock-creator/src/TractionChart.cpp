#include "TractionChart.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QPen>
#include <QPolygonF>

#include <algorithm>
#include <cmath>

//------------------------------------------------------------------------------
//
//  График тяговой характеристики: отрисовка QPainter.
//
//------------------------------------------------------------------------------
namespace
{

/// «Красивый» шаг сетки для диапазона: 1/2/5 x 10^n,
/// чтобы линий получилось примерно target_count
double niceStep(double range, int target_count)
{
    if (range <= 0.0 || target_count <= 0)
    {
        return 1.0;
    }

    const double rough = range / target_count;
    const double magnitude = std::pow(10.0, std::floor(std::log10(rough)));
    const double normalized = rough / magnitude;

    double nice = 10.0;

    if (normalized < 1.5)
    {
        nice = 1.0;
    }
    else if (normalized < 3.5)
    {
        nice = 2.0;
    }
    else if (normalized < 7.5)
    {
        nice = 5.0;
    }

    return nice * magnitude;
}

/// Компактная подпись числа для оси
QString axisLabel(double value)
{
    return QString::number(value, 'g', 4);
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TractionChart::TractionChart(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(320, 220);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TractionChart::setPoints(const std::vector<Point>& points)
{
    points_ = points;

    // Полилиния строится по возрастанию скорости
    std::sort(points_.begin(), points_.end());

    update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TractionChart::setAxisTitles(const QString& x_title, const QString& y_title)
{
    x_title_ = x_title;
    y_title_ = y_title;
    update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TractionChart::setEmptyText(const QString& text)
{
    empty_text_ = text;
    update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QSize TractionChart::minimumSizeHint() const
{
    return QSize(320, 220);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TractionChart::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Фон
    painter.fillRect(rect(), palette().brush(QPalette::Base));

    const QFontMetrics metrics(painter.font());

    // Поля под подписи осей
    const int margin_left = metrics.horizontalAdvance(QStringLiteral("-1000")) + 12;
    const int margin_bottom = metrics.height() + 14;
    const int margin_top = metrics.height() + 10;
    const int margin_right = 12;

    const QRect plot_area(margin_left, margin_top,
                          width() - margin_left - margin_right,
                          height() - margin_top - margin_bottom);

    if (plot_area.width() < 20 || plot_area.height() < 20)
    {
        return;
    }

    // Диапазоны осей: по данным, с запасом до «красивого» шага
    double max_speed = 100.0;
    double max_force = 100.0;

    if (!points_.empty())
    {
        max_speed = std::max(1.0,
            std::max_element(points_.begin(), points_.end())->first);
        max_force = 1.0;

        for (const Point& point : points_)
        {
            max_force = std::max(max_force, point.second);
        }
    }

    const double step_v = niceStep(max_speed, 5);
    const double step_f = niceStep(max_force, 5);

    const double axis_max_v = std::ceil(max_speed / step_v) * step_v;
    const double axis_max_f = std::ceil(max_force / step_f) * step_f;

    auto x_of = [&plot_area, axis_max_v](double speed) -> double
    {
        return plot_area.left() +
               plot_area.width() * speed / axis_max_v;
    };

    auto y_of = [&plot_area, axis_max_f](double force) -> double
    {
        return plot_area.bottom() -
               plot_area.height() * force / axis_max_f;
    };

    // Сетка и подписи
    QPen grid_pen(palette().brush(QPalette::Mid).color(), 1, Qt::DotLine);
    painter.setPen(grid_pen);

    QPen text_pen(palette().brush(QPalette::Text).color());
    painter.setFont(font());

    for (double v = 0.0; v <= axis_max_v + 0.5 * step_v; v += step_v)
    {
        const int x = static_cast<int>(std::lround(x_of(v)));

        if (v > 0.0)
        {
            painter.drawLine(x, plot_area.top(), x, plot_area.bottom());
        }

        painter.setPen(text_pen);
        painter.drawText(QRect(x - 30, plot_area.bottom() + 4, 60,
                               metrics.height()),
                         Qt::AlignHCenter | Qt::AlignTop,
                         axisLabel(v));
        painter.setPen(grid_pen);
    }

    for (double f = 0.0; f <= axis_max_f + 0.5 * step_f; f += step_f)
    {
        const int y = static_cast<int>(std::lround(y_of(f)));

        if (f > 0.0)
        {
            painter.drawLine(plot_area.left(), y, plot_area.right(), y);
        }

        painter.setPen(text_pen);
        painter.drawText(QRect(plot_area.left() - margin_left + 2, y -
                               metrics.height() / 2,
                               margin_left - 6, metrics.height()),
                         Qt::AlignRight | Qt::AlignVCenter,
                         axisLabel(f));
        painter.setPen(grid_pen);
    }

    // Оси
    QPen axis_pen(palette().brush(QPalette::Text).color(), 1);
    painter.setPen(axis_pen);
    painter.drawLine(plot_area.left(), plot_area.top(),
                     plot_area.left(), plot_area.bottom());
    painter.drawLine(plot_area.left(), plot_area.bottom(),
                     plot_area.right(), plot_area.bottom());

    // Подписи осей и заголовок
    painter.drawText(QRect(plot_area.left(), height() - metrics.height() - 2,
                           plot_area.width(), metrics.height()),
                     Qt::AlignRight | Qt::AlignVCenter,
                     x_title_);
    painter.drawText(QRect(plot_area.left(), 2, plot_area.width(),
                           metrics.height()),
                     Qt::AlignHCenter | Qt::AlignVCenter,
                     y_title_);

    // Полилиния и точки характеристики
    if (!points_.empty())
    {
        QPen line_pen(palette().brush(QPalette::Link).color(), 2);
        painter.setPen(line_pen);
        painter.setBrush(palette().brush(QPalette::Link));

        QPolygonF polyline;

        for (const Point& point : points_)
        {
            polyline.append(QPointF(x_of(point.first), y_of(point.second)));
        }

        painter.drawPolyline(polyline);

        for (const QPointF& vertex : polyline)
        {
            painter.drawEllipse(vertex, 3.0, 3.0);
        }
    }
    else
    {
        painter.setPen(QPen(palette().brush(QPalette::PlaceholderText).color()));
        painter.drawText(plot_area,
                         Qt::AlignCenter,
                         empty_text_);
    }
}
