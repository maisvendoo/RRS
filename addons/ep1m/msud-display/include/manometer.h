#ifndef MANOMETER_H
#define MANOMETER_H

#include <QLabel>

#include    "manometer-arrow.h"



class Manometer : public QLabel
{
public:
    Manometer(QSize _size, QString manName, int maxValScale,
              QWidget* parent = Q_NULLPTR, bool _flag = true, QString labelColor = "yellow");

    void addTxtLab1(QRect geo);
    void addTxtLab2(QRect geo);

    void setVal_Line(int val);
    void setVal_Arrow(int val);

    void setVal_zonaVIP(double val);

    void setLabelColor(const QString &color) { labColor = color; }

private:
    QString familyFont_;

    ManometerArrow* manArrows_;


    QLabel* labVal1_;
    QLabel* labVal2_;

    QString labColor;


    void createLab_(QLabel* &lab, QRect geo);

};

#endif // MANOMETER_H
