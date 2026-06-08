#include    "mainwindow.h"

#include    <core/load_module.h>

#include    <QVBoxLayout>

MainWindow::MainWindow(QString& module_path, QString& config_path, QWidget* parent) : QWidget(parent)
{
    display = LOAD_MODULE(AbstractDisplay, module_path);
    display->setConfigDir(config_path);
    display->init();

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->resize(display->size());

    this->setAutoFillBackground(true);
    this->setPalette(QPalette(QColor(255, 255, 255)));

    QLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->addWidget(display);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);
}
