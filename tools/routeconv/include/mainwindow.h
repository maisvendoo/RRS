#ifndef     MAINWINDOW_H
#define     MAINWINDOW_H

#include    <QMainWindow>
#include    <QProcess>



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
namespace Ui
{
    class MainWindow;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private:

    Ui::MainWindow *ui;

    QString routesRootDir;

    QString routeDir;

    QProcess    pathconvProc;
    QProcess    profconvProc;

    bool createRouteTypeFile();

    bool createDescriptionFile(QString title, QString description);

    void startPathConverter(QString routeDir);

    void startProfConverter(QString routeDir);

private slots:

    void slotOpenRoute();

    void slotConvert();

    void slotIsPathconvFinished(int error_code);

    void slotIsProfconvFinished(int error_code);
};

#endif // MAINWINDOW_H
