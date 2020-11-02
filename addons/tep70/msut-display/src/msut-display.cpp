#include    "msut-display.h"

#include    <QLayout>
#include    <QLabel>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MsutDisplay::MsutDisplay(QWidget *parent, Qt::WindowFlags f)
    : AbstractDisplay(parent, f)
{
    this->setWindowFlag(Qt::WindowType::FramelessWindowHint);
    this->resize(644, 465);
    this->setAutoFillBackground(true);
    this->setPalette(QPalette(QColor(0, 0, 0)));
    this->setAttribute(Qt::WA_TransparentForMouseEvents);

    this->setLayout(new QVBoxLayout);
    this-> setFocusPolicy(Qt::FocusPolicy::NoFocus);
    this->layout()->setContentsMargins(0, 0, 0, 0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MsutDisplay::~MsutDisplay()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MsutDisplay::init()
{
    QLabel *background = new QLabel(this);
    background->setFrameShape(QLabel::NoFrame);

    QPixmap pic;

    if (!pic.load(":/msut/msut-main"))
    {
        return;
    }

    background->setFixedWidth(pic.width());
    background->setFixedHeight(pic.height());

    background->setPixmap(pic);

    this->layout()->addWidget(background);

    AbstractDisplay::init();
}

GET_DISPLAY(MsutDisplay);
