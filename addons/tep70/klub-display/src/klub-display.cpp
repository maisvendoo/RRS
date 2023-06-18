#include    "klub-display.h"

#include    <QLabel>
#include    <QLayout>

#include    "tep70bs-signals.h"

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
    update_timer.setInterval(500);
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

    // Лампа проверки бдительности (треугольная)
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
    if (!TO_BOOL(input_signals[KLUB_ON]))
        return;

    alarm->setState(TO_BOOL(input_signals[KLUB_ALARM]));
}

GET_DISPLAY(KlubDisplay)
