#ifndef SCALEARROW_H
#define SCALEARROW_H

#include <QLabel>

class ScaleArrow : public QLabel
{
public:
    ScaleArrow(QSize _size, int otstupSverhu, QWidget* parent = Q_NULLPTR);

    void setIsArrow(bool flag);
    void setMaxVal(double val);

    void setVal(double val);

private:
    double w_2_;
    double h_2_;

    double cpX_;
    double cpY_;

    QImage img_;

    bool isArrow_;
    double maxVal_;



    void draw_(double _val);

};

#endif // SCALEARROW_H
