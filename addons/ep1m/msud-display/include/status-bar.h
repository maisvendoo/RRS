#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QLabel>



class StatusBar : public QLabel
{
public:
    StatusBar(QSize _size, int maxVal, double EPS, QWidget *parent = Q_NULLPTR);

    double setVal(double val);

private:
    QImage img_;

    int maxVal_;
    int oldVal_;
    double EPS_;



    void drawBar_(double val);

};

#endif // STATUSBAR_H
