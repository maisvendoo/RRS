#ifndef REVERSEIND_H
#define REVERSEIND_H


#include <QLabel>



class ReverseInd : public QLabel
{

public:
    ReverseInd(QSize _size, QWidget *parent = Q_NULLPTR);
    void setReverse(int val);


private:
    int oldVal_ = -2;

    void drawReverse_(int val);

};

#endif // REVERSEIND_H
