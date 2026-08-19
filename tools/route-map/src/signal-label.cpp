#include    <signal-label.h>
#include    <QMouseEvent>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SignalLabel::SignalLabel(QWidget* parent) : QLabel(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SignalLabel::~SignalLabel()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SignalLabel::resetMenu()
{
    menu = nullptr;
    action_open_train = nullptr;
    action_open_shunting = nullptr;
    action_open_call = nullptr;
    action_close = nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SignalLabel::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        emit popUpMenu(signal);
    }
}
