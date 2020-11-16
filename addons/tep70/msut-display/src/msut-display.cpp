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


    // Текущая дата
    labelCurDate_ = new QLabel(background);
    labelCurDate_->setFont(QFont("Arial", 10, 57));
    labelCurDate_->setText(QDate::currentDate().toString("dd.MM.yyyy"));
    labelCurDate_->setStyleSheet("color: black;");
    labelCurDate_->move(18, 5);

    // Текущее время
    labelCurTime_ = new QLabel(background);
    labelCurTime_->setFont(QFont("Arial", 10, 57));
    labelCurTime_->setText(QTime::currentTime().toString());
    labelCurTime_->setStyleSheet("color: black;");
    labelCurTime_->move(558, 5);



    // ЭТ/Тяга
    scaleArrow_ = new ScaleArrow(QSize(220, 100), 28, background);
    scaleArrow_->setIsArrow(true);
    scaleArrow_->setMaxVal(60);
    scaleArrow_->move(307, 145);
    //scaleArrow_->setStyleSheet("border: 2px solid red;");
    scaleArrow_->setVal(0.00 + 30.0);

    labelArrow_ = new QLabel(this);
    drawNumberLabel_(labelArrow_, QRect(376,220, 30,20), 12, "#000080", Qt::AlignRight);
    //labelArrow_->setText("120");


    // Скорость
    scaleSpeed_ = new ScaleArrow(QSize(94, 90), 40, background);
    scaleSpeed_->setMaxVal(200);
    scaleSpeed_->move(312, 40);
    //scaleSpeed_->setStyleSheet("border: 2px solid red;");
    scaleSpeed_->setVal(0.0);

    labelSpeed_ = new QLabel(this);
    drawNumberLabel_(labelSpeed_, QRect(315,105, 30,15), 10, "black", Qt::AlignRight);
    //labelSpeed_->setText("120");


    // Ускорение
    scaleAcceleration_ = new ScaleArrow(QSize(94, 90), 40, background);
    scaleAcceleration_->setMaxVal(4);
    scaleAcceleration_->move(427, 40);
    //scaleAcceleration_->setStyleSheet("border: 2px solid red;");
    scaleAcceleration_->setVal(0.0 + 2.0);

    labelAcceleration_ = new QLabel(this);
    drawNumberLabel_(labelAcceleration_, QRect(428,105, 30,15), 10, "black", Qt::AlignRight);
    //labelAcceleration_->setText("120");


    // ВУ1
    //
    frameVU1_Ited_ = new QFrame(background);
    frameVU1_Ited_->setAutoFillBackground(true);
    frameVU1_Ited_->setPalette(QPalette(QColor("#000080")));
    frameVU1_Ited_->resize(13, 0);
    frameVU1_Ited_->move(179, 68);

    labelVU1_Ited_ = new QLabel(background);
    drawNumberLabel_(labelVU1_Ited_, QRect(150,314, 50,20), 14, "#000080");
    labelVU1_Ited_->setText("0");

    //
    frameVU1_I_ = new QFrame(background);
    frameVU1_I_->setAutoFillBackground(true);
    frameVU1_I_->setPalette(QPalette(QColor("#800080")));
    frameVU1_I_->resize(13, 0);
    frameVU1_I_->move(229, 68);

    labelVU1_I_ = new QLabel(background);
    drawNumberLabel_(labelVU1_I_, QRect(204,314, 50,20), 14, "#800080");
    labelVU1_I_->setText("0");

    //
    frameVU1_U_ = new QFrame(background);
    frameVU1_U_->setAutoFillBackground(true);
    frameVU1_U_->setPalette(QPalette(QColor("#000080")));
    frameVU1_U_->resize(13, 0);
    frameVU1_U_->move(280, 68);

    labelVU1_U_ = new QLabel(background);
    drawNumberLabel_(labelVU1_U_, QRect(258,314, 50,20), 14, "#000080");
    labelVU1_U_->setText("0");

    // ВУ1
    //
    frameVU2_U_ = new QFrame(background);
    frameVU2_U_->setAutoFillBackground(true);
    frameVU2_U_->setPalette(QPalette(QColor("#000080")));
    frameVU2_U_->resize(13, 0);
    frameVU2_U_->move(540, 68);

    labelVU2_U_ = new QLabel(background);
    drawNumberLabel_(labelVU2_U_, QRect(525,314, 50,20), 14, "#000080");
    labelVU2_U_->setText("0");

    //
    frameVU2_I_ = new QFrame(background);
    frameVU2_I_->setAutoFillBackground(true);
    frameVU2_I_->setPalette(QPalette(QColor("#800080")));
    frameVU2_I_->resize(13, 0);
    frameVU2_I_->move(596, 68);

    labelVU2_I_ = new QLabel(background);
    drawNumberLabel_(labelVU2_I_, QRect(582,314, 50,20), 14, "#800080");
    labelVU2_I_->setText("0");




    // kW
    //
    label_kW_left_ = new QLabel(background);
    drawNumberLabel_(label_kW_left_, QRect(327,256, 50,20), 14, "#000080", Qt::AlignLeft);
    label_kW_left_->setText("0");

    //
    label_kW_right_ = new QLabel(background);
    drawNumberLabel_(label_kW_right_, QRect(456,256, 50,20), 14, "#800080", Qt::AlignRight);
    label_kW_right_->setText("0");

    //
    hBar_ = new HorizontBar(QSize(202, 25), background);
    hBar_->move(316, 284);



    // РЕВЕРСОР
    labelReversorFwd_ = new QLabel(background);
    labelReversorFwd_->move(55, 48);
    //labelReversorFwd_->setStyleSheet("border: 1px solid blue");
    QPixmap picReversorArrowFwd;
    picReversorArrowFwd.load(":/msut/reversor-arrow-fwd");
    labelReversorFwd_->setPixmap(picReversorArrowFwd);
    labelReversorFwd_->setVisible(false);

    labelReversorBwd_ = new QLabel(background);
    labelReversorBwd_->move(55, 48);
    //labelReversorBwd_->setStyleSheet("border: 1px solid blue");
    QPixmap picReversorArrowBwd;
    picReversorArrowBwd.load(":/msut/reversor-arrow-bwd");
    labelReversorBwd_->setPixmap(picReversorArrowBwd);
    labelReversorBwd_->setVisible(false);


    // ПОЗИЦИЯ
    labelPositin_ = new QLabel(background);
    drawNumberLabel_(labelPositin_, QRect(35,146, 80,48), 35, "white");
    labelPositin_->setText("0");

    // РЕЖИМ
    labelRezim_ = new QLabel(background);
    drawNumberLabel_(labelRezim_, QRect(15,244, 115,45), 14, "white");
    labelRezim_->setText("СТОП");




    //
    this->layout()->addWidget(background);

    AbstractDisplay::init();
}



void MsutDisplay::drawNumberLabel_(QLabel* lab, QRect geo, int fontSize, QString color, Qt::Alignment align)
{
    lab->resize(geo.size());
    lab->move(geo.x(), geo.y());
    //lab->setStyleSheet("border: 1px solid red;");
    lab->setStyleSheet("color:"+ color +";");
    lab->setAlignment(align);
    lab->setFont(QFont("Arial", fontSize, 63));
}



void MsutDisplay::slotUpdateTimer()
{
    labelCurDate_->setText(QDate::currentDate().toString("dd.MM.yyyy"));
    labelCurTime_->setText(QTime::currentTime().toString());

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


    scaleArrow_->setVal(30.0 + input_signals[MSUT_ET_T]);
    labelArrow_->setText(QString::number(input_signals[MSUT_ET_T]));
    scaleSpeed_->setVal(input_signals[MSUT_SPEED]);
    labelSpeed_->setText(QString::number(input_signals[MSUT_SPEED]));
    scaleAcceleration_->setVal(2.0 + input_signals[MSUT_ACCELLERATION]);
    labelAcceleration_->setText(QString::number(input_signals[MSUT_ACCELLERATION]));


    int fooH = 231;
    int fooW = 13;
    int fooY0 = 68;
    // для виду. Удалить. zБогос
    /*input_signals[MSUT_VU1_I_TED] = 1.2;
    input_signals[MSUT_VU1_I] = 7;
    input_signals[MSUT_VU1_U] = 0.8;
    input_signals[MSUT_VU2_U] = 3.2;
    input_signals[MSUT_VU2_I] = 200;*/

    frameVU1_Ited_->resize(fooW, fooH*input_signals[MSUT_VU1_I_TED]/1.5);
    frameVU1_Ited_->move(frameVU1_Ited_->x(), fooY0 + fooH*(1.5 - input_signals[MSUT_VU1_I_TED])/1.5);
    labelVU1_Ited_->setText(QString::number(qRound(input_signals[MSUT_VU1_I_TED] * 1000)));
    frameVU1_I_->resize(fooW, fooH*input_signals[MSUT_VU1_I]/8);
    frameVU1_I_->move(frameVU1_I_->x(), fooY0 + fooH*(8 - input_signals[MSUT_VU1_I])/8);
    labelVU1_I_->setText(QString::number(qRound(input_signals[MSUT_VU1_I] * 1000)));
    frameVU1_U_->resize(fooW, fooH*input_signals[MSUT_VU1_U]);
    frameVU1_U_->move(frameVU1_U_->x(), fooY0 + fooH*(1 - input_signals[MSUT_VU1_U]));
    labelVU1_U_->setText(QString::number(qRound(input_signals[MSUT_VU1_U] * 1000)));
    frameVU2_U_->resize(fooW, fooH*input_signals[MSUT_VU2_U]/4);
    frameVU2_U_->move(frameVU2_U_->x(), fooY0 + fooH*(4 - input_signals[MSUT_VU2_U])/4);
    labelVU2_U_->setText(QString::number(qRound(input_signals[MSUT_VU2_U] * 1000)));
    frameVU2_I_->resize(fooW, fooH*input_signals[MSUT_VU2_I]/250);
    frameVU2_I_->move(frameVU2_I_->x(), fooY0 + fooH*(250 - input_signals[MSUT_VU2_I])/250);
    labelVU2_I_->setText(QString::number(qRound(input_signals[MSUT_VU2_I] * 1000)));

    label_kW_left_->setText(QString::number(qRound(input_signals[MSUT_POWER] - input_signals[MSUT_POWER_OTOPLENIE])));
    label_kW_right_->setText(QString::number(qRound(input_signals[MSUT_POWER_OTOPLENIE])));


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

    labelPositin_->setText(QString::number(static_cast<int>(input_signals[MSUT_POSITION])));

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

    default:
        labelRezim_->setText("СТОП");
    }

}



GET_DISPLAY(MsutDisplay);
