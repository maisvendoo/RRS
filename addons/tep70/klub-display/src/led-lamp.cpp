#include    "led-lamp.h"


LEDLamp::LEDLamp(QWidget *parent) : Label(parent)
  , state(false)
  , old_state(false)  
{
    this->setAlignment(Qt::AlignCenter);
}

LEDLamp::~LEDLamp()
{

}

void LEDLamp::setOnImage(QString path)
{
    pic_on.load(path);
}

void LEDLamp::setOffImage(QString path)
{
    pic_off.load(path);
    this->setPixmap(&pic_off);

    width = pic_off.width();
    height = pic_off.height();
}

void LEDLamp::setState(bool state)
{
    this->old_state = this->state;
    this->state = state;

    if (this->old_state != this->state)
    {
        if (this->state)
            this->setPixmap(&pic_on);
        else
            this->setPixmap(&pic_off);

        this->update();
    }
}

void LEDLamp::setPosition(int x, int y)
{
    QRect rect(x - width / 2, y - height / 2, width, height);
    this->setGeometry(rect);
}
