#include    <mainwindow.h>
#include    <./ui_mainwindow.h>
#include    <QFileDialog>
#include    <QDir>
#include    <QFileInfo>
#include    <filesystem.h>

const QString TEXCOMPRESS_NAME = "texcompr";

#ifdef __WIN32__
    const QString EXE_EXP = ".exe";
#else
    const QString EXE_EXP = "";
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pbOpenModel, &QPushButton::released, this, &MainWindow::slotOpenModel);
    connect(ui->pbAddTexture, &QPushButton::released, this, &MainWindow::slotAddSkipedTexture);
    connect(ui->pbCompress, &QPushButton::released, this, &MainWindow::slotCopmpress);

    // Объединяем stdout и stderr в один поток вывода
    texCompressor->setProcessChannelMode(QProcess::MergedChannels);
    connect(texCompressor, &QProcess::readyRead, this, &MainWindow::slotOnReadyReadStdout);
    connect(texCompressor, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::slotOnCompressionFinish);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOpenModel()
{
    modelFilePath.clear();

    FileSystem &fs = FileSystem::getInstance();
    QString startDir = QString(fs.getDataDir().c_str()) + fs.separator() + "models";

    if (!QDir(startDir).exists())
    {
        startDir = QDir::homePath();
    }

    QString filter = tr("Models glTF 2.0 (*.gltf)");

    modelFilePath = QFileDialog::getOpenFileName(nullptr,
                                                 tr("Select model file"),
                                                 startDir,
                                                 filter);

    modelFilePath = QDir::toNativeSeparators(modelFilePath);

    ui->teModelPath->setText(modelFilePath);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotAddSkipedTexture()
{
    if (modelFilePath.isEmpty())
    {
        return;
    }

    QFileInfo fileInfo(modelFilePath);

    QString startDir = fileInfo.absoluteFilePath();

    QString filter = tr("Image files (*.png *.jpg *.bmp *.tga)");

    QString texturePath = QFileDialog::getOpenFileName(nullptr,
                                                       tr("Select texture file"),
                                                       startDir,
                                                       filter);

    if (texturePath.isEmpty())
    {
        return;
    }

    QFileInfo textureInfo(texturePath);

    ui->lwSkipedTextures->addItem(textureInfo.fileName());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotDeleteSkipedTexture()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotCopmpress()
{
    if (modelFilePath.isEmpty())
    {
        return;
    }

    FileSystem &fs = FileSystem::getInstance();
    QString workDir = QString(fs.getBinaryDir().c_str());

    QStringList args;
    args << "-m" << modelFilePath;

    if (ui->cbMipMaps->isChecked())
    {
        args << "-g";
    }

    if (ui->cbOverwriteGLTF->isChecked())
    {
        args << "-o";
    }

    QString texComprPath = workDir + QDir::separator() + TEXCOMPRESS_NAME + EXE_EXP;

    texCompressor->setWorkingDirectory(workDir);
    texCompressor->start(texComprPath, args);

    ui->pbCompress->setEnabled(false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnReadyReadStdout()
{
    QByteArray data = texCompressor->readAllStandardOutput();
    QString text = QString::fromUtf8(data);

    ui->ptLog->appendPlainText(text);

    QTextCursor cursor = ui->ptLog->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->ptLog->setTextCursor(cursor);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnCompressionFinish(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString statusText;

    if (exitStatus == QProcess::NormalExit)
    {
        statusText = QString(tr("Execution success. Exit code %1")).arg(exitCode);
    }
    else
    {
        statusText = QString(tr("Execution failed. Exit code %1")).arg(exitCode);
    }

    ui->ptLog->appendPlainText(statusText);
    ui->pbCompress->setEnabled(true);
}
