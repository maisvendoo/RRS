#ifndef     IMAGE_LABEL_H
#define     IMAGE_LABEL_H

#include    <QLabel>
#include    <QPainter>

class ImageLabel : public QLabel
{
public:

    ImageLabel(QWidget *parent = Q_NULLPTR) : QLabel(parent)
      , pixmap(Q_NULLPTR)
    {

    }

    ImageLabel(const QString &text, QWidget *parent = Q_NULLPTR) : QLabel(text, parent)
      , pixmap(Q_NULLPTR)
    {

    }

    virtual ~ImageLabel()
    {

    }

    void setPixmap(QPixmap *pxmp)
    {
        pixmap = pxmp;
    }


private:

    QPixmap     *pixmap;

    void paintEvent(QPaintEvent *)
    {
        if (pixmap == Q_NULLPTR)
            return;

        QPainter p(this);
        p.setWindow(this->frameGeometry());
        p.drawPixmap(this->frameGeometry(), *pixmap);
    }
};


#endif // IMAGE_LABEL_H
