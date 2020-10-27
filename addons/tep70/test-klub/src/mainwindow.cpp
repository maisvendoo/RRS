#include    "mainwindow.h"

#include    <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : display(Q_NULLPTR)
{
    display = loadDisplay("../modules/tep70bs/klub-display");
    display->init();

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->resize(2048, 638);
    this->setAutoFillBackground(true);
    this->setPalette(QPalette(QColor(255, 255, 255)));

    QLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->addWidget(display);
    layout->setContentsMargins(0, 0, 0, 0);

    this->setLayout(layout);
}

MainWindow::~MainWindow()
{

}
