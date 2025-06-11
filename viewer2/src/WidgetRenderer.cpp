#include "WidgetRenderer.h"

#include <QImage>
#include <QObject>
#include <QPainter>
#include <QThread>
#include <QWidget>

WidgetRenderer::WidgetRenderer(QWidget* widget, QObject* parent)
    : QObject(parent)
    , widget(widget)
{
}

QImage WidgetRenderer::renderToImage()
{
    if (QThread::currentThread() != widget->thread())
    {
        QImage image;
        QMetaObject::invokeMethod(this, "renderInGuiThread", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QImage, image));
        return image;
    }
    else
    {
        return renderWidgetToImage();
    }
}

void WidgetRenderer::update()
{
    QImage image = renderToImage();
    emit imageUpdated(image);
}

QImage WidgetRenderer::renderInGuiThread()
{
    return renderWidgetToImage();
}

QImage WidgetRenderer::renderWidgetToImage()
{
    if (!widget)
    {
        return QImage();
    }

    QImage image(widget->size(), QImage::Format_ARGB32_Premultiplied); // Другой формат???
    image.fill(Qt::transparent);

    QPainter painter(&image);
    widget->render(&painter);

    return image;
}
