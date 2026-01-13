#include    <train-label.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainLabel::TrainLabel(QWidget *parent)
{

}

TrainLabel::~TrainLabel()
{

}

void TrainLabel::resetMenu()
{
    menu = nullptr;
    action_rename = nullptr;
}

void TrainLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        emit popUpMenu();
    }
}

