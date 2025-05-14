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
    QProcess    dmd2gltfProc;
    QProcess    parallelGenProc;
    QProcess    splineGenProc;

    bool createRouteTypeFile();

    bool createDescriptionFile(QString title, QString description);

    void startPathConverter(QString routeDir);

    void startProfConverter(QString routeDir);

    void startDmd2gltfConverter(QString routeDir);

    void startParallelGenerator(QString routeDir);

    void startSplineGenerator(QString routeDir);

private slots:

    void slotOpenRoute();

    void slotConvert();

    void slotIsPathconvFinished(int error_code, QProcess::ExitStatus exitstatus);

    void slotIsProfconvFinished(int error_code, QProcess::ExitStatus exitstatus);

    void slotIsDmd2gltfFinished(int error_code, QProcess::ExitStatus exitstatus);

    void slotGenerateParallel();

    void slotGenerateSpline();
};

#endif // MAINWINDOW_H
