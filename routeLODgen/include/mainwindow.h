#ifndef     MAINWINDOW_H
#define     MAINWINDOW_H

#include    <QMainWindow>
#include    <QMap>
#include    <QTimer>

#include    "object-data.h"

QT_BEGIN_NAMESPACE
    namespace Ui { class MainWindow; }
QT_END_NAMESPACE

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private:

    Ui::MainWindow *ui;

    QString routesRootDir;

    QString routeDir;

    size_t clean_count;

    QTimer cleanTimer;

    QMap<QString, object_data_t> objects_ref;

    QMap<QString, QString> used_models;

    std::vector<object_data_t> unused_models;

    void objectRefReader();

    void findUsedModels();

private slots:


    void slotOnQuit();

    void slotOnRouteOpen();

    void slotOnCleanRoute();

    void slotOnCleanTimer();
};

#endif // MAINWINDOW_H
