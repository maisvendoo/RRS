#ifndef     LED_LAMP_H
#define     LED_LAMP_H

#include    "label.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class LEDLamp : public Label
{
public:

    LEDLamp(QWidget *parent = Q_NULLPTR);

    ~LEDLamp();

    void setOnImage(QString path);

    void setOffImage(QString path);

    void setState(bool state);

    void setPosition(int x, int y);

private:

    bool state;

    bool old_state;


    int width;

    int height;

    QPixmap pic_on;

    QPixmap pic_off;

    QPixmap pic_cur;
};

#endif // LED_LAMP_H
