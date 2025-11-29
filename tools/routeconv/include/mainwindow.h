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

    QString outputDir;

    QProcess    pathconvProc;
    QProcess    profconvProc;
    QProcess    dmd2gltfProc;
    QProcess    topologyCheckProc;
    QProcess    parallelGenProc;
    QProcess    splineGenProc;

    bool createRouteTypeFile();

    bool createDescriptionFile(QString title, QString description);

    bool loadDescriptionFile(QString route_dir);

    void startPathConverter();

    void startProfConverter();

    void startDmd2gltfConverter();

    void startTopologyChecker();

    void startParallelGenerator();

    void startSplineGenerator();

private slots:

    void slotOpenRoute();

    void slotSelectOutputPath();

    void slotConvert();

    void slotIsPathconvFinished(int error_code, QProcess::ExitStatus exitstatus);

    void slotIsProfconvFinished(int error_code, QProcess::ExitStatus exitstatus);

    void slotIsDmd2gltfFinished(int error_code, QProcess::ExitStatus exitstatus);

    void slotCheckTopology();

    void slotGenerateParallel();

    void slotGenerateSpline();
};

#endif // MAINWINDOW_H
