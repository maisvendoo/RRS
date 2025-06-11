#ifndef WIDGET_RENDERER_H
#define WIDGET_RENDERER_H

#include <QImage>
#include <QObject>

class QWidget;

class WidgetRenderer : public QObject
{
Q_OBJECT
public:
    explicit WidgetRenderer(QWidget* widget, QObject* parent = nullptr);

    QImage renderToImage();

signals:
    void imageUpdated(const QImage& image);

public slots:
    void update();

private slots:
    QImage renderInGuiThread();

private:
    QImage renderWidgetToImage();

private:
    QWidget* widget;
};

#endif // WIDGET_RENDERER_H
