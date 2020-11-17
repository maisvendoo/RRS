#ifndef SCALEARROW2_H
#define SCALEARROW2_H

#include <QLabel>

class ScaleArrow2 : public QLabel
{
public:
    ScaleArrow2(QSize _size, int otstupSverhu, QWidget* parent = Q_NULLPTR);

    void setMaxVal(double val);

    void setVal(double val);

private:
    double w_2_;
    double h_2_;

    double cpX_;
    double cpY_;

    QImage img_;

    double maxVal_;



    void draw_(double _val);

};

#endif // SCALEARROW2_H
