#include    "msut-display.h"

#include    "tep70-signals.h"

#include    <QLayout>
#include    <QLabel>
#include    <QPainter>
#include    <QtCore/qmath.h>
#include    <QDate>


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
    this->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    this->layout()->setContentsMargins(0, 0, 0, 0);


    connect(&update_timer, &QTimer::timeout, this, &MsutDisplay::slotUpdateTimer);
    update_timer.setInterval(500);
    update_timer.start();


    timerCount_ = 10;
    connect(&timerObratniyOtschet_, &QTimer::timeout,
            this, &MsutDisplay::slotTimerObratniyOtschet_);
    timerObratniyOtschet_.setInterval(1000);
    //timerObratniyOtschet_.start();




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
    //
    background_ = new QLabel(this);
    background_->setFrameShape(QLabel::NoFrame);
    if (!pic_.load(":/msut/msut-main-move")) { return; }
    background_->setFixedSize(pic_.size());
    background_->setPixmap(pic_);



    // Текущая дата
    labelCurDate_ = new QLabel(background_);
    labelCurDate_->setFont(QFont("Arial", 10, 57));
    labelCurDate_->setText(QDate::currentDate().toString("dd.MM.yyyy"));
    labelCurDate_->setStyleSheet("color: black;");
    labelCurDate_->move(18, 5);

    // Текущее время
    labelCurTime_ = new QLabel(background_);
    labelCurTime_->setFont(QFont("Arial", 10, 57));
    labelCurTime_->setText(QTime::currentTime().toString());
    labelCurTime_->setStyleSheet("color: black;");
    labelCurTime_->move(558, 5);



    // Элементы основного экрана. Движение
    msutMainDispMove_ = new MsutMainDispMove(background_);

    // Элементы основного экрана. Стоянка
    msutMainDispParking_ = new MsutMainDispParking(background_);
    msutMainDispParking_->setMyVisible(false);




    // РЕВЕРСОР
    labelReversorFwd_ = new QLabel(background_);
    labelReversorFwd_->move(55, 48);
    //labelReversorFwd_->setStyleSheet("border: 1px solid blue");
    QPixmap picReversorArrowFwd;
    picReversorArrowFwd.load(":/msut/reversor-arrow-fwd");
    labelReversorFwd_->setPixmap(picReversorArrowFwd);
    labelReversorFwd_->setVisible(false);

    labelReversorBwd_ = new QLabel(background_);
    labelReversorBwd_->move(55, 48);
    //labelReversorBwd_->setStyleSheet("border: 1px solid blue");
    QPixmap picReversorArrowBwd;
    picReversorArrowBwd.load(":/msut/reversor-arrow-bwd");
    labelReversorBwd_->setPixmap(picReversorArrowBwd);
    labelReversorBwd_->setVisible(false);


    // ПОЗИЦИЯ
    labelPositin_ = new QLabel(background_);
    drawNumberLabel_(labelPositin_, QRect(35,146, 80,48), 35);
    labelPositin_->setText("0");

    // ПОЗИЦИЯ/ВРЕМЯ
    label_Positin_Time_ = new QLabel(background_);
    label_Positin_Time_->resize(105, 20);
    label_Positin_Time_->move(20, 198);
    //label_Positin_Time_->setStyleSheet("border: 1px solid red; color: red;");
    label_Positin_Time_->setStyleSheet("color: #000080;");
    label_Positin_Time_->setAlignment(Qt::AlignHCenter);
    label_Positin_Time_->setFont(QFont("Arial", 12, 0, true));
    label_Positin_Time_->setText("ПОЗИЦИЯ");



    // РЕЖИМ
    labelRezim_ = new QLabel(background_);
    drawNumberLabel_(labelRezim_, QRect(15,244, 115,45), 14);
    labelRezim_->setText("СТОП");




    //
    this->layout()->addWidget(background_);

    AbstractDisplay::init();
}



void MsutDisplay::drawNumberLabel_(QLabel* lab, QRect geo, int fontSize, QString color, Qt::Alignment align)
{
    lab->resize(geo.size());
    lab->move(geo.x(), geo.y());
    lab->setStyleSheet("color:"+ color +";");
    lab->setAlignment(align);
    lab->setFont(QFont("Arial", fontSize, 63));
}



void MsutDisplay::slotUpdateTimer()
{
//    if (    (!TO_BOOL(input_signals[MSUT_SPEED])) ||
//            (!TO_BOOL(input_signals[MSUT_ACCELLERATION])) ||
//            (!TO_BOOL(input_signals[MSUT_ET_T])) ||
//            (!TO_BOOL(input_signals[MSUT_REVERSOR])) ||
//            (!TO_BOOL(input_signals[MSUT_POSITION])) ||
//            (!TO_BOOL(input_signals[MSUT_MODE])) ||
//            (!TO_BOOL(input_signals[MSUT_VU1_I_TED])) ||
//            (!TO_BOOL(input_signals[MSUT_VU1_I])) ||
//            (!TO_BOOL(input_signals[MSUT_VU1_U])) ||
//            (!TO_BOOL(input_signals[MSUT_VU2_U])) ||
//            (!TO_BOOL(input_signals[MSUT_VU2_I])) ||
//            (!TO_BOOL(input_signals[MSUT_POWER]))       )
//        return;





    //
    int z = static_cast<int>(input_signals[MSUT_NOMER_KADR_DISP]);
    if (z == 1)
    {
        msutMainDispMove_->setMyVisible();
        msutMainDispParking_->setMyVisible(false);
        if (!pic_.load(":/msut/msut-main-move")) { return; }

        msutMainDispMove_->updateData(input_signals);
    }
    else
    if (z == 2)
    {
        msutMainDispMove_->setMyVisible(false);
        msutMainDispParking_->setMyVisible();
        if (!pic_.load(":/msut/msut-main-parking")) { return; }

        msutMainDispParking_->updateData(input_signals);
    }
    else
    {
        return;
    }

    background_->setPixmap(pic_);



    //
    labelCurDate_->setText(QDate::currentDate().toString("dd.MM.yyyy"));
    labelCurTime_->setText(QTime::currentTime().toString());




    switch (static_cast<int>(input_signals[MSUT_REVERSOR]))
    {
    case 0:
        labelReversorFwd_->setVisible(false);
        labelReversorBwd_->setVisible(false);
        break;
    case 1:
        labelReversorFwd_->setVisible(true);
        labelReversorBwd_->setVisible(false);
        break;
    case -1:
        labelReversorFwd_->setVisible(false);
        labelReversorBwd_->setVisible(true);
        break;

    default:
        labelReversorFwd_->setVisible(false);
        labelReversorBwd_->setVisible(false);
    }


    if ( (static_cast<int>(input_signals[MSUT_MODE]) == 5) ||
         (static_cast<int>(input_signals[MSUT_MODE]) == 6) ||
         (static_cast<int>(input_signals[MSUT_MODE]) == 7) )
    {
        label_Positin_Time_->setText("ВРЕМЯ");
    }
    else
    {
        label_Positin_Time_->setText("ПОЗИЦИЯ");
        labelPositin_->setText(QString::number(static_cast<int>(input_signals[MSUT_POSITION])));
    }


    switch (static_cast<int>(input_signals[MSUT_MODE]))
    {
    case 0:
        labelRezim_->setText("СТОП");
        break;
    case 1:
        labelRezim_->setText("ТЯГА");
        break;
    case 2:
        labelRezim_->setText("ВЫБЕГ");
        break;
    case 3:
        labelRezim_->setText("ЭДТ");
        break;
    case 4:
        labelRezim_->setText("ЗАМЕЩЕНИЕ");
        break;
    case 5:
        labelRezim_->setText("ПРОКАЧКА");
        timerCount_ = static_cast<int>(input_signals[MSUT_TIMER_PROKACHKA]);
        if (!timerObratniyOtschet_.isActive())
            timerObratniyOtschet_.start();
        break;
    case 6:
        labelRezim_->setText("ПРОКРУТКА");
        timerCount_ = static_cast<int>(input_signals[MSUT_TIMER_PROKRUTKA]);
        if (!timerObratniyOtschet_.isActive())
            timerObratniyOtschet_.start();
        break;
    case 7:
        labelRezim_->setText("ОСТАНОВ");
        timerCount_ = static_cast<int>(input_signals[MSUT_TIMER_OSTANOV]);
        if (!timerObratniyOtschet_.isActive())
            timerObratniyOtschet_.start();
        break;
    case 8:
        labelRezim_->setText("ХОЛ. ХОД");
        break;

    default:
        labelRezim_->setText("СТОП");
    }

}

void MsutDisplay::slotTimerObratniyOtschet_()
{
    labelPositin_->setText(QString::number(timerCount_));

    if (timerCount_ <= 0)
        timerObratniyOtschet_.stop();

    --timerCount_;
}



//void MsutDisplay::slotTimerProkachka_()
//{

//}

//void MsutDisplay::slotTimerProkrutka_()
//{

//}

//void MsutDisplay::slotTimerOstanov_()
//{

//}



GET_DISPLAY(MsutDisplay);
