#ifndef TRACTION_CHART_H
#define TRACTION_CHART_H

#include <QWidget>

#include <utility>
#include <vector>

//------------------------------------------------------------------------------
//
//  Превью-график тяговой характеристики F(v): скорость, км/ч — по
//  горизонтали, сила тяги, кН — по вертикали. Рисуется QPainter:
//  оси, сетка, полилиния по точкам, подписи. При отсутствии точек —
//  пустая сетка (секция <TractiveCurve> конфига).
//
//------------------------------------------------------------------------------
class TractionChart : public QWidget
{
public:
    /// Точка характеристики: first — скорость, км/ч; second — сила, кН
    using Point = std::pair<double, double>;

    explicit TractionChart(QWidget* parent = nullptr);

    /// Задать точки (сортируются по скорости) и перерисовать
    void setPoints(const std::vector<Point>& points);

    /// Задать подписи осей (по умолчанию «v, км/ч» / «F, кН»);
    /// используется для переиспользования графика под другие данные
    /// (например, профили загрузки, промт п.19)
    void setAxisTitles(const QString& x_title, const QString& y_title);

    /// Задать текст при отсутствии точек
    /// (по умолчанию — подсказка про секцию TractiveCurve)
    void setEmptyText(const QString& text);

    /// Минимальный размер виджета
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<Point> points_; ///< Точки характеристики (по возрастанию v)

    QString x_title_ = QStringLiteral("v, км/ч");  ///< Подпись оси X
    QString y_title_ = QStringLiteral("F, кН");    ///< Подпись оси Y
    QString empty_text_ = QStringLiteral(          ///< Текст без данных
        "Добавьте точки характеристики\n(секция TractiveCurve)");
};

#endif // TRACTION_CHART_H
