#ifndef     MAINWINDOW_H
#define     MAINWINDOW_H

#include    <QMainWindow>
#include    <QProcess>

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

    QProcess *texCompressor = new QProcess(this);

private slots:


    void slotOpenModel();

    void slotAddSkipedTexture();

    void slotDeleteSkipedTexture();

    void slotCopmpress();

    void slotOnReadyReadStdout();

    void slotOnCompressionFinish(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // MAINWINDOW_H
