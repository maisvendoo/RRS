#ifndef     MAINWINDOW_H
#define     MAINWINDOW_H

#include    <QMainWindow>
#include    <QProcess>
#include    <QQueue>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
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

    QString modelFilePath = "";

    QString modelsDirPath = "";

    QProcess *texCompressor = new QProcess(this);

    QProcess *texExtractor = new QProcess(this);

    // Состояние задачи
    QQueue<QString> m_fileQueue;
    QList<QProcess*> m_runningProcesses;
    int m_totalFiles = 0;
    int m_processedFiles = 0;
    int m_maxConcurrency = 1;

    QString lastDir = "";

    QStringList scanGltfFiles(const QString& dir);

    QString extractedModelPath = "";

    void launchNextProcess();

    void updateProgress();

    void onProcessFinished(QProcess* proc, int exitCode, QProcess::ExitStatus exitStatus);

    void onProcessErrorOccurred(QProcess::ProcessError error);

private slots:


    void slotOpenModel();

    void slotAddSkipedTexture();

    void slotDeleteSkipedTexture();

    void slotCopmpress();

    void slotOnReadyReadStdout();

    void slotOnCompressionFinish(int exitCode, QProcess::ExitStatus exitStatus);

    void slotSaveSkipList();

    void slotLoadSkipList();

    void slotOpenDirectory();

    void slotDirectoryCompress();

    void slotOpenExtractedModel();

    void slotExtractModelsTextures();
};

#endif // MAINWINDOW_H
