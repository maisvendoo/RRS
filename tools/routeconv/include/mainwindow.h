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

    QString status;
    QString subProcessStatus;

    QProcess    pathconvProc;
    QProcess    profconvProc;
    QProcess    dmd2gltfProc;
    QProcess    topologyCheckProc;
    QProcess    transformRouteProc;
    QProcess    parallelGenProc;
    QProcess    splineGenProc;

    double latitude = 47.2;
    double longitude = 39.7;

    bool createRouteTypeFile();

    bool createDescriptionFile(QString title, QString description);

    bool loadDescriptionFile(QString route_dir);

    void startPathConverter();

    void startProfConverter();

    void startDmd2gltfConverter();

    void startTopologyChecker();

    void startTransformRoute();

    void startParallelGenerator();

    void startSplineGenerator();

    void updateStatus();
    void updateStatus(QString new_status, QString new_subprocess_status = "");

    int getDMDConversionPercent(QString sub_status);

    void loadSettingsGUI();

private slots:

    void slotSubProcessReadyReadStandardError();

    void slotOpenRoute();

    void slotSelectOutputPath();

    void slotConvert();

    void slotIsPathconvFinished(int error_code, QProcess::ExitStatus exitstatus);

    void slotIsProfconvFinished(int error_code, QProcess::ExitStatus exitstatus);

    void slotIsDmd2gltfFinished(int error_code, QProcess::ExitStatus exitstatus);

    void slotCheckTopology();

    void slotTransformRoute();

    void slotGenerateParallel();

    void slotGenerateSpline();
};

#endif // MAINWINDOW_H
