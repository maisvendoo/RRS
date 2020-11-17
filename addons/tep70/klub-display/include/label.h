#ifndef     LABEL_H
#define     LABEL_H

#include    <QLabel>
#include    <QPainter>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Label : public QLabel
{
public:

    Label(QWidget *parent = Q_NULLPTR) : QLabel(parent)
      , pixmap(Q_NULLPTR)
    {

    }

    ~Label()
    {

    }

    void setPixmap(QPixmap *pixmap)
    {
        this->pixmap = pixmap;
    }

private:

    QPixmap *pixmap;

    void paintEvent(QPaintEvent *)
    {
        if (pixmap == Q_NULLPTR)
            return;

        QPainter p(this);
        p.setWindow(this->frameGeometry());
        p.drawPixmap(this->frameGeometry(), *pixmap);
    }
};

#endif
