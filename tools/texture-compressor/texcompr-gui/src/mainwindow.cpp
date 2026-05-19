#include    <mainwindow.h>
#include    <./ui_mainwindow.h>
#include    <QFileDialog>
#include    <QDir>
#include    <QFileInfo>
#include    <QFile>
#include    <filesystem.h>
#include    <QDirIterator>
#include    <QThread>
#include    <CfgReader.h>
#include    <styles.h>

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

    loadSettingsGUI();

    connect(ui->pbOpenModel, &QPushButton::released, this, &MainWindow::slotOpenModel);
    connect(ui->pbAddTexture, &QPushButton::released, this, &MainWindow::slotAddSkipedTexture);
    connect(ui->pbDeleteTexture, &QPushButton::released, this, &MainWindow::slotDeleteSkipedTexture);

    connect(ui->pbClear, &QPushButton::released, this, [this](){
        ui->lwSkipedTextures->clear();
    });

    connect(ui->pbCompress, &QPushButton::released, this, &MainWindow::slotCopmpress);

    // Объединяем stdout и stderr в один поток вывода
    texCompressor->setProcessChannelMode(QProcess::MergedChannels);
    connect(texCompressor, &QProcess::readyRead, this, &MainWindow::slotOnReadyReadStdout);
    connect(texCompressor, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::slotOnCompressionFinish);

    texExtractor->setProcessChannelMode(QProcess::MergedChannels);
    connect(texExtractor, &QProcess::readyRead, this, &MainWindow::slotOnReadyReadStdout);
    connect(texExtractor, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::slotOnCompressionFinish);

    connect(ui->pbSave, &QPushButton::released, this, &MainWindow::slotSaveSkipList);
    connect(ui->pbLoad, &QPushButton::released, this, &MainWindow::slotLoadSkipList);

    ui->lwSkipedTextures->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui->pbOpenFolder, &QPushButton::released, this, &MainWindow::slotOpenDirectory);

    ui->prbCompression->setValue(0);

    m_maxConcurrency = qMax(1, QThread::idealThreadCount());

    connect(ui->pbCompressDir, &QPushButton::released, this, &MainWindow::slotDirectoryCompress);

    connect(ui->pbOpenModelForExtract, &QPushButton::released, this, &MainWindow::slotOpenExtractedModel);
    connect(ui->pbExtract, &QPushButton::released, this, &MainWindow::slotExtractModelsTextures);
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
void MainWindow::loadSettingsGUI()
{
    FileSystem &fs = FileSystem::getInstance();
    std::string cfg_dir = fs.getConfigDir();
    std::string cfg_path = fs.combinePath(cfg_dir, "gui-settings.xml");

    CfgReader cfg;

    if ( cfg.load(QString(cfg_path.c_str())) )
    {
        QString secName = "GUISettings";
        QString theme_name = "";

        if (!cfg.getString(secName, "Theme", theme_name))
        {
            theme_name = "dark-jedy";
        }

        std::string theme_dir = fs.getThemeDir();
        std::string theme_path = fs.combinePath(theme_dir, theme_name.toStdString() + ".qss");
        QString style_sheet = readStyleSheet(QString(theme_path.c_str()));

        this->setStyleSheet(style_sheet);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QStringList MainWindow::scanGltfFiles(const QString &dir)
{
    QStringList files;
    QDirIterator it(dir, {"*.gltf", "*.gltf2"}, QDir::Files, QDirIterator::Subdirectories);

    while (it.hasNext())
    {
        files.append(it.next());
    }

    files.sort();
    return files;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::launchNextProcess()
{
    FileSystem &fs = FileSystem::getInstance();
    QString workDir(fs.getBinaryDir().c_str());

    QString texComprPath = workDir + QDir::separator() + TEXCOMPRESS_NAME + EXE_EXP;

    while (m_runningProcesses.size() < m_maxConcurrency && !m_fileQueue.isEmpty())
    {
        QString filePath = m_fileQueue.dequeue();
        QProcess* proc = new QProcess(this);

        // Настройка параметров из help.txt
        // -m обязательно, остальные опциональны. Можно добавить флаги через UI.
        proc->setProgram(texComprPath);

        QStringList args;

        args << "-m" << QDir::toNativeSeparators(filePath);

        if (ui->cbGenMipmapsDir->isChecked())
        {
            args << "-g";
        }

        if (ui->cbOverWriteGltfDir->isChecked())
        {
            args << "-o";
        }

        if (ui->cbNoRewriteKtxDir->isChecked())
        {
            args << "-i";
        }

        proc->setArguments(args);

        // Асинхронные соединения
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, proc](int exitCode, QProcess::ExitStatus exitStatus) {
                    onProcessFinished(proc, exitCode, exitStatus);
                });

        connect(proc, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
            onProcessErrorOccurred(error);
        });

        proc->start();

        if (proc->state() == QProcess::Starting || proc->state() == QProcess::Running)
        {
            m_runningProcesses.append(proc);
            ui->ptDirLog->appendPlainText(QString(tr("Started: %1")).arg(QFileInfo(filePath).fileName()));
        }
        else
        {
            ui->ptDirLog->appendPlainText(QString(tr("Failed start: %1")).arg(filePath));
            proc->deleteLater();

            // Если запуск не удался, сразу уменьшаем счётчик и пробуем запустить следующий
            m_processedFiles++;

            updateProgress();

            launchNextProcess();
        }
    }

    if (m_fileQueue.isEmpty() && m_runningProcesses.isEmpty())
    {
        ui->pbCompressDir->setEnabled(true);
        ui->ptDirLog->appendPlainText(tr("All tasks finished success"));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::onProcessFinished(QProcess* proc, int exitCode, QProcess::ExitStatus exitStatus)
{
    m_runningProcesses.removeAll(proc);
    m_processedFiles++;
    updateProgress();

    QString fileName = QFileInfo(proc->arguments().value(1)).fileName();
    if (exitStatus == QProcess::NormalExit)
    {
        ui->ptDirLog->appendPlainText(QString(tr("Ready: %1")).arg(fileName));
    }
    else
    {
        ui->ptDirLog->appendPlainText(QString(tr("Error: %1 (exit code: %2, state: %3)"))
                                          .arg(fileName)
                                          .arg(exitCode)
                                          .arg(exitStatus == QProcess::NormalExit ? "Normal" : "Crash"));
    }

    proc->deleteLater();
    launchNextProcess(); // Запускаем следующие из очереди
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::onProcessErrorOccurred(QProcess::ProcessError error)
{
    QProcess* proc = qobject_cast<QProcess*>(sender());

    if (!proc)
    {
        return;
    }

    Q_UNUSED(error);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::updateProgress()
{
    ui->prbCompression->setValue(qRound(100.0 * m_processedFiles / m_totalFiles));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOpenModel()
{
    modelFilePath.clear();

    FileSystem &fs = FileSystem::getInstance();

    QString startDir = "";

    if (lastDir.isEmpty())
    {
        startDir = QString(fs.getDataDir().c_str()) + fs.separator() + "models";

        if (!QDir(startDir).exists())
        {
            startDir = QDir::homePath();
        }
    }
    else
    {
        startDir = lastDir;
    }

    QString filter = tr("Models glTF 2.0 (*.gltf)");

    modelFilePath = QFileDialog::getOpenFileName(nullptr,
                                                 tr("Select model file"),
                                                 startDir,
                                                 filter);

    modelFilePath = QDir::toNativeSeparators(modelFilePath);
    QFileInfo fileInfo(modelFilePath);
    lastDir = fileInfo.absoluteFilePath();

    ui->teModelPath->setText(modelFilePath);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotAddSkipedTexture()
{
    if (ui->teModelPath->toPlainText().isEmpty())
    {
        return;
    }

    QFileInfo fileInfo(modelFilePath);

    QString startDir = fileInfo.absoluteFilePath();

    QString filter = tr("Image files (*.png *.jpg *.bmp *.tga)");

    QStringList texturePaths = QFileDialog::getOpenFileNames(nullptr,
                                                             tr("Select texture file"),
                                                             startDir,
                                                             filter);

    if (texturePaths.isEmpty())
    {
        return;
    }

    for (const QString& tex_info : texturePaths)
    {
        QFileInfo textureInfo(tex_info);
        ui->lwSkipedTextures->addItem(textureInfo.fileName());
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotDeleteSkipedTexture()
{
    if (ui->lwSkipedTextures->count() == 0)
    {
        return;
    }

    int cur_idx = ui->lwSkipedTextures->currentRow();

    if (cur_idx == -1)
    {
        return;
    }

    auto* item = ui->lwSkipedTextures->takeItem(cur_idx);
    delete item;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotCopmpress()
{
    if (ui->teModelPath->toPlainText().isEmpty())
    {
        return;
    }

    modelFilePath = ui->teModelPath->toPlainText();

    FileSystem &fs = FileSystem::getInstance();
    QString workDir = QString(fs.getBinaryDir().c_str());

    QStringList args;
    args << "-m" << ui->teModelPath->toPlainText();

    if (ui->cbMipMaps->isChecked())
    {
        args << "-g";
    }

    if (ui->cbOverwriteGLTF->isChecked())
    {
        args << "-o";
    }

    if (ui->cbNoRewrite)
    {
        args << "-i";
    }

    if (ui->cbDeleteSrcTex->isChecked())
    {
        args << "-d";
    }

    if (ui->lwSkipedTextures->count() != 0)
    {
        args << "-s";

        QString tex_list = "";

        for (int i = 0; i < ui->lwSkipedTextures->count(); ++i)
        {
            tex_list += ui->lwSkipedTextures->item(i)->text();

            if (i < ui->lwSkipedTextures->count() - 1)
            {
                tex_list += ",";
            }
        }

        args << tex_list;
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
    auto* proc = dynamic_cast<QProcess *>(sender());

    if (proc == texCompressor)
    {
        QByteArray data = proc->readAllStandardOutput();
        QString text = QString::fromUtf8(data).remove("\n");

        ui->ptLog->appendPlainText(text);

        QTextCursor cursor = ui->ptLog->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->ptLog->setTextCursor(cursor);
    }

    if (proc == texExtractor)
    {
        QByteArray data = proc->readAllStandardOutput();
        QString text = QString::fromUtf8(data).remove("\n");

        ui->ptExtractLog->appendPlainText(text);

        QTextCursor cursor = ui->ptExtractLog->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->ptExtractLog->setTextCursor(cursor);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOnCompressionFinish(int exitCode, QProcess::ExitStatus exitStatus)
{
    auto* proc = dynamic_cast<QProcess *>(sender());

    QString statusText = "";

    if (proc == texCompressor)
    {
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

    if (proc == texExtractor)
    {
        if (exitStatus == QProcess::NormalExit)
        {
            statusText = QString(tr("Execution success. Exit code %1")).arg(exitCode);
        }
        else
        {
            statusText = QString(tr("Execution failed. Exit code %1")).arg(exitCode);
        }

        ui->ptExtractLog->appendPlainText(statusText);
        ui->pbExtract->setEnabled(true);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSaveSkipList()
{
    if (modelFilePath.isEmpty())
    {
        return;
    }

    QFileInfo fileInfo(modelFilePath);

    QString startDir = fileInfo.absoluteFilePath();

    QString filter = tr("Text files (*.txt)");

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"), startDir, filter);

    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }

    QTextStream out(&file);

    for (int i = 0; i < ui->lwSkipedTextures->count(); ++i)
    {
        out << ui->lwSkipedTextures->item(i)->text() << "\n";
    }

    file.close();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotLoadSkipList()
{
    if (modelFilePath.isEmpty())
    {
        return;
    }

    QFileInfo fileInfo(modelFilePath);

    QString startDir = fileInfo.absoluteFilePath();

    QString filter = tr("Text files (*.txt)");

    QString fileName = QFileDialog::getOpenFileName(nullptr,
                                                    tr("Select text file"),
                                                    startDir,
                                                    filter);

    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    ui->lwSkipedTextures->clear();

    while (!file.atEnd())
    {
        QString line = file.readLine();
        QListWidgetItem *item = new QListWidgetItem(ui->lwSkipedTextures);
        item->setText(line);
        ui->lwSkipedTextures->addItem(item);
    }

    file.close();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOpenDirectory()
{
    modelsDirPath.clear();

    FileSystem &fs = FileSystem::getInstance();
    QString startDir = QString(fs.getLevelUpDirectory(fs.getBinaryDir(), 1).c_str());

    modelsDirPath = QFileDialog::getExistingDirectory(this, tr("Select models folder"), startDir);

    if (modelsDirPath.isEmpty())
    {
        return;
    }

    modelsDirPath = QDir::toNativeSeparators(modelsDirPath);

    ui->teDirectoryPath->setText(modelsDirPath);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotDirectoryCompress()
{
    QString dir = modelsDirPath.trimmed();

    if (dir.isEmpty() || !QDir(dir).exists())
    {
        return;
    }

    m_fileQueue.clear();
    qDeleteAll(m_runningProcesses);
    m_runningProcesses.clear();
    m_totalFiles = 0;
    m_processedFiles = 0;
    ui->ptDirLog->clear();

    QStringList gltfFiles = scanGltfFiles(dir);

    if (gltfFiles.empty())
    {
        ui->ptDirLog->appendPlainText(tr("There are no GLTF files. Abort"));
        return;
    }

    m_totalFiles = gltfFiles.size();

    for (const QString &f : gltfFiles)
    {
        m_fileQueue.enqueue(f);
    }

    ui->pbCompressDir->setEnabled(false);
    ui->prbCompression->setValue(0);
    ui->ptDirLog->appendPlainText(QString(tr("Find %1 files for compression...")).arg(m_totalFiles));

    launchNextProcess();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotOpenExtractedModel()
{
    extractedModelPath.clear();

    FileSystem &fs = FileSystem::getInstance();

    QString startDir = "";

    if (lastDir.isEmpty())
    {
        startDir = QString(fs.getDataDir().c_str()) + fs.separator() + "models";

        if (!QDir(startDir).exists())
        {
            startDir = QDir::homePath();
        }
    }
    else
    {
        startDir = lastDir;
    }

    QString filter = tr("Models glTF 2.0 (*.gltf)");

    extractedModelPath = QFileDialog::getOpenFileName(nullptr,
                                                 tr("Select model file"),
                                                 startDir,
                                                 filter);

    extractedModelPath = QDir::toNativeSeparators(extractedModelPath);
    QFileInfo fileInfo(extractedModelPath);
    lastDir = fileInfo.absoluteFilePath();

    ui->teExtractedModelPath->setText(extractedModelPath);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotExtractModelsTextures()
{
    if (ui->teExtractedModelPath->toPlainText().isEmpty())
    {
        return;
    }

    extractedModelPath = ui->teExtractedModelPath->toPlainText();

    FileSystem &fs = FileSystem::getInstance();
    QString workDir = QString(fs.getBinaryDir().c_str());

    QStringList args;
    args << "-m" << ui->teExtractedModelPath->toPlainText();
    args << "-e";

    QString texComprPath = workDir + QDir::separator() + TEXCOMPRESS_NAME + EXE_EXP;

    texExtractor->setWorkingDirectory(workDir);
    texExtractor->start(texComprPath, args);

    ui->pbExtract->setEnabled(false);
}
