#include    "blok-display.h"

BlokDisplay::BlokDisplay(QWidget *parent, Qt::WindowFlags f)
    : AbstractDisplay(parent, f)

{
    this->setWindowFlag(Qt::WindowType::FramelessWindowHint);
    this->resize(1024, 768);
    this->setAutoFillBackground(true);
    this->setPalette(QPalette(QColor(1, 0, 0)));
}

BlokDisplay::~BlokDisplay()
{

}

GET_DISPLAY(BlokDisplay)
