#ifndef TEXTPAINT_H
#define TEXTPAINT_H

#include <QLabel>



class TextPaint : public QLabel
{

public:
    TextPaint(QSize _size, QWidget *parent = Q_NULLPTR,
              Qt::Alignment align = Qt::AlignCenter);

    void setText(QString str);

private:

    void drawText_(QString txt);


};

#endif // TEXTPAINT_H
