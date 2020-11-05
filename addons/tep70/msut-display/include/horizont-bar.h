#ifndef HORIZONTBAR_H
#define HORIZONTBAR_H

#include <QLabel>


class HorizontBar : public QLabel
{
public:
    HorizontBar(QSize _size, QWidget* parent = Q_NULLPTR);


private:
    QImage img_;

    void draw_();

};

#endif // HORIZONTBAR_H
