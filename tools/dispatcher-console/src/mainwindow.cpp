#include    <mainwindow.h>

#include    <CfgReader.h>
#include    <filesystem.h>

#include    <QApplication>
#include    <QStatusBar>
#include    <QVBoxLayout>
#include    <QTimer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{
    this->setWindowTitle(tr("Dispatcher Console"));

    resize(1280, 800);

    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);

    ptLog = new QPlainTextEdit(this);
    ptLog->setReadOnly(true);
    ptLog->setMaximumBlockCount(1000);

    layout->addWidget(ptLog);

    setCentralWidget(central);

    lblStatus = new QLabel(tr("Disconnected"));
    statusBar()->addPermanentWidget(lblStatus);

    tcp_client = new TcpClient(this);

    connect(tcp_client, &TcpClient::connected,
            this, &MainWindow::slotConnected);

    connect(tcp_client, &TcpClient::disconnected,
            this, &MainWindow::slotDisconnected);

    connect(tcp_client, &TcpClient::sendLogMessage,
            this, &MainWindow::slotRecvLogMessage);

    loadSettingsGUI();

    loadConfig("../cfg/dispatcher-console-tcp.xml");

    QTimer::singleShot(100, tcp_client, &TcpClient::slotConnect);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    if (tcp_client != nullptr)
    {
        disconnect(tcp_client, nullptr, this, nullptr);
        tcp_client->slotDisconnect();
    }
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
        QString style_sheet;

        QFile file(QString(theme_path.c_str()));
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            style_sheet = file.readAll();
            file.close();
        }

        if (!style_sheet.isEmpty())
        {
            this->setStyleSheet(style_sheet);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::loadConfig(const QString &cfg_name)
{
    CfgReader cfg;

    if (!cfg.load(cfg_name))
    {
        ptLog->appendPlainText(tr("Config not found, use defaults"));
        return;
    }

    QString secName = "Client";
    cfg.getString(secName, "HostAddr", tcp_config.host_addr);

    int tmp = 0;
    if (cfg.getInt(secName, "port", tmp))
    {
        tcp_config.port = static_cast<quint16>(tmp);
    }

    cfg.getBool(secName, "ShowServerAddr", tcp_config.show_server_addr);

    cfg.getInt(secName, "ReconnectInteval", tcp_config.reconnect_interval);

    ptLog->appendPlainText(tr("Config loaded: %1:%2")
                               .arg(tcp_config.host_addr)
                               .arg(tcp_config.port));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::updateStatus(const QString &text)
{
    if (lblStatus != nullptr)
    {
        lblStatus->setText(text);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotConnected()
{
    ptLog->appendPlainText(tr("Connected to simulator"));
    updateStatus(tr("Connected"));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotDisconnected()
{
    ptLog->appendPlainText(tr("Disconnected from simulator"));
    updateStatus(tr("Disconnected"));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotRecvLogMessage(QString msg)
{
    ptLog->appendPlainText(msg);
}