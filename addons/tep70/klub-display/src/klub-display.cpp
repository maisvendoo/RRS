#include    "klub-display.h"

#include    <QLabel>
#include    <QLayout>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
KlubDisplay::KlubDisplay(QWidget *parent, Qt::WindowFlags f) : AbstractDisplay(parent, f)
  , alarm_state(false)
{
    this->setWindowFlag(Qt::WindowType::FramelessWindowHint);
    this->resize(2048, 638);
    this->setAutoFillBackground(true);
    this->setPalette(QPalette(QColor(255, 255, 255)));
    this->setAttribute(Qt::WA_TransparentForMouseEvents);

    this->setLayout(new QVBoxLayout);
    this-> setFocusPolicy(Qt::FocusPolicy::NoFocus);
    this->layout()->setContentsMargins(0, 0, 0, 0);

    connect(&update_timer, &QTimer::timeout, this, &KlubDisplay::slotUpdateTimer);
    update_timer.setInterval(100);
    update_timer.start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
KlubDisplay::~KlubDisplay()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KlubDisplay::init()
{
    QLabel *background = new QLabel(this);
    background->setFrameShape(QLabel::NoFrame);

    QPixmap pic;

    if (!pic.load(":/klub/img-background"))
    {
        return;
    }

    background->setFixedWidth(pic.width());
    background->setFixedHeight(pic.height());

    background->setPixmap(pic);

    background->setAttribute(Qt::WA_TransparentForMouseEvents);

    alarm = new LEDLamp(background);
    alarm->setOnImage(":/klub/alarm_on");
    alarm->setOffImage(":/klub/alarm_off");
    alarm->setPosition(1935, 137);

    this->layout()->addWidget(background);



    AbstractDisplay::init();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KlubDisplay::slotUpdateTimer()
{
    //alarm->setState(true);
    alarm->setState(alarm_state);

    alarm_state = !alarm_state;
}

GET_DISPLAY(KlubDisplay)
