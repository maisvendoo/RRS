#ifndef MANOMETERARROW_H
#define MANOMETERARROW_H


#include <QLabel>



class ManometerArrow : public QLabel
{
public:
    ManometerArrow(QSize _size, int maxValScale, QWidget* parent = Q_NULLPTR);

//    QPair<int , int> setVals(int val1_line, int val2_arrow);


    int setVal_Line(int val);
    int setVal_Arrow(int val);

    double setVal_LineD(double val);



private:
    double w_2_;
    double h_2_;

    QImage img_;

    int maxValScale_;

    int oldVal1_line_;
    int oldVal2_arrow_;
    double oldVal1_lineD_;


    void drawArrow_(double val1_line, int val2_arrow = -1);


};

#endif // MANOMETERARROW_H
