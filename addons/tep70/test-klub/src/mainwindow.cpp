#include    "mainwindow.h"

#include    <QVBoxLayout>

MainWindow::MainWindow(QString module_path, QWidget *parent) : QWidget(parent)
    , display(Q_NULLPTR)
{
    display = loadDisplay(module_path);
    display->init();

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->resize(display->width(), display->height());

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
