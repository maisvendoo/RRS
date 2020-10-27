#include    "klub-display.h"

#include    <QLabel>
#include    <QLayout>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
KlubDisplay::KlubDisplay(QWidget *parent, Qt::WindowFlags f)
{
    this->setWindowFlag(Qt::WindowType::FramelessWindowHint);
    this->resize(2048, 638);
    this->setAutoFillBackground(true);
    this->setPalette(QPalette(QColor(255, 255, 255)));

    this->setLayout(new QVBoxLayout);
    this-> setFocusPolicy(Qt::FocusPolicy::NoFocus);
    this->layout()->setContentsMargins(0, 0, 0, 0);
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

    this->layout()->addWidget(background);

    AbstractDisplay::init();
}

GET_DISPLAY(KlubDisplay)
