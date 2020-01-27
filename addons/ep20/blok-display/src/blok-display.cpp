#include    "blok-display.h"

#include    <QVBoxLayout>

BlokDisplay::BlokDisplay(QWidget *parent, Qt::WindowFlags f)
    : AbstractDisplay(parent, f)

{
    this->setWindowFlag(Qt::WindowType::FramelessWindowHint);
    this->resize(1024, 768);
    this->setAutoFillBackground(true);
    this->setPalette(QPalette(QColor(255, 0, 0)));

    this->setLayout(new QVBoxLayout);
    test = new QLabel;
    //test->setText("Привет!!!");
    test->setAlignment(Qt::AlignCenter);
    QPalette palette;
    palette.setColor(QPalette::WindowText, QColor(255, 255, 0));
    test->setPalette(palette);
    QFont font = test->font();
    font.setPointSize(72);
    test->setFont(font);
    this->layout()->addWidget(test);

    updateTimer = new QTimer;
    updateTimer->setInterval(100);
    connect(updateTimer, &QTimer::timeout, this, &BlokDisplay::slotUpdateTimer);
    updateTimer->start();
}

BlokDisplay::~BlokDisplay()
{

}

void BlokDisplay::slotUpdateTimer()
{
    test->setText(QString("ТЦ: %1 МПа").arg(input_signals[100], 4, 'f', 2));
}

GET_DISPLAY(BlokDisplay)
