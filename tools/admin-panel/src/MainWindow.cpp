//------------------------------------------------------------------------------
//
//  Main window
//  (c) SimulatorClient 2026
//
//------------------------------------------------------------------------------

#include    "MainWindow.h"
#include    "ui_mainwindow.h"
#include    "CfgReader.h"
#include    <QMessageBox>
#include    <QCloseEvent>
#include    <QDebug>
#include    <QCoreApplication>
#include    <QDir>
#include    <QFile>

//-----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_client(new ClientCore(this))
    , m_isConnected(false)
    , m_isSimulationRunning(false)
{
    ui->setupUi(this);

    // Подключение сигналов UI
    connect(ui->btnConnect, &QPushButton::clicked,
            this, &MainWindow::onConnectButtonClicked);
    connect(ui->listRoutes, &QListWidget::itemClicked,
            this, &MainWindow::onRouteSelected);
    connect(ui->btnStart, &QPushButton::clicked,
            this, &MainWindow::onStartButtonClicked);
    connect(ui->comboScenarios, &QComboBox::currentIndexChanged,
            this, &MainWindow::onScenarioChanged);

    // Подключение сигналов клиента
    connect(m_client, &ClientCore::connected,
            this, &MainWindow::onConnected);
    connect(m_client, &ClientCore::disconnected,
            this, &MainWindow::onDisconnected);
    connect(m_client, &ClientCore::error,
            this, &MainWindow::onError);

    connect(m_client, &ClientCore::routesLoaded,
            this, &MainWindow::onRoutesLoaded);
    connect(m_client, &ClientCore::scenariosLoaded,
            this, &MainWindow::onScenariosLoaded);
    connect(m_client, &ClientCore::statusUpdated,
            this, &MainWindow::onStatusUpdated);
    connect(m_client, &ClientCore::simulationStarted,
            this, &MainWindow::onSimulationStarted);
    connect(m_client, &ClientCore::simulationStopped,
            this, &MainWindow::onSimulationStopped);

    // Загрузка конфига
    loadConfig();

    setStatus("Disconnected", true);
    updateUI();
}

//-----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}

//-----------------------------------------------------------------------------
void MainWindow::loadConfig()
{
    // Путь к конфигу: ../cfg/admin-panel.xml
    QString basePath = QCoreApplication::applicationDirPath();
    QDir baseDir(basePath);
    baseDir.cdUp();
    QString configPath = baseDir.absolutePath() + "/cfg/admin-panel.xml";
    
    qDebug() << "Looking for config:" << configPath;
    
    if (!QFile::exists(configPath))
    {
        qWarning() << "Config not found:" << configPath;
        qWarning() << "Using default values: 127.0.0.1:12345";
        return;
    }
    
    qDebug() << "Config found:" << configPath;
    
    CfgReader cfg;
    if (!cfg.load(configPath))
    {
        qWarning() << "Failed to load config:" << configPath;
        return;
    }
    
    QDomNode clientNode = cfg.getFirstSection("Client");
    if (clientNode.isNull())
    {
        qWarning() << "Client section not found in config";
        return;
    }
    
    // Чтение хоста
    QString host;
    if (cfg.getString(clientNode, "DefaultHost", host))
    {
        if (!host.isEmpty())
        {
            ui->editHost->setText(host);
            qDebug() << "Loaded host:" << host;
        }
    }
    
    // Чтение порта
    int port;
    if (cfg.getInt(clientNode, "DefaultPort", port))
    {
        if (port > 0 && port < 65536)
        {
            ui->editPort->setText(QString::number(port));
            qDebug() << "Loaded port:" << port;
        }
    }
}

//-----------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_isConnected)
    {
        m_client->disconnectFromServer();
    }
    event->accept();
}

//-----------------------------------------------------------------------------
void MainWindow::onConnectButtonClicked()
{
    if (m_isConnected)
    {
        m_client->disconnectFromServer();
        return;
    }

    QString host = ui->editHost->text().trimmed();
    if (host.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please enter host address");
        return;
    }

    bool ok;
    int port = ui->editPort->text().toInt(&ok);
    if (!ok || port <= 0 || port > 65535)
    {
        QMessageBox::warning(this, "Error", "Please enter valid port number");
        return;
    }

    ui->btnConnect->setEnabled(false);
    setStatus("Connecting...");

    if (!m_client->connectToServer(host, static_cast<quint16>(port)))
    {
        setStatus("Connection failed", true);
        ui->btnConnect->setEnabled(true);
        QMessageBox::warning(this, "Error", "Failed to connect to server");
    }
}

//-----------------------------------------------------------------------------
void MainWindow::onRouteSelected(QListWidgetItem* item)
{
    if (!item)
        return;

    QString routeName = item->data(Qt::UserRole).toString();
    if (routeName.isEmpty())
        return;

    m_currentRoute = routeName;
    loadScenarios(routeName);

    // Обновляем информацию о маршруте
    for (const RouteData& route : m_client->getRoutes())
    {
        if (route.name == routeName)
        {
            updateRouteInfo(route);
            break;
        }
    }
}

//-----------------------------------------------------------------------------
void MainWindow::onStartButtonClicked()
{
    if (!m_isConnected)
    {
        QMessageBox::warning(this, "Error", "Not connected to server");
        return;
    }

    if (m_isSimulationRunning)
    {
        m_client->stopSimulation();
        return;
    }

    if (m_currentRoute.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please select a route");
        return;
    }

    if (m_currentScenario.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please select a scenario");
        return;
    }

    // Получаем dirName сценария (имя каталога)
    int index = ui->comboScenarios->currentIndex();
    if (index < 0)
    {
        return;
    }
    
    QString scenarioDirName = ui->comboScenarios->itemData(index, Qt::UserRole + 1).toString();
    if (scenarioDirName.isEmpty())
    {
        scenarioDirName = m_currentScenario;
    }

    qDebug() << "Sending start simulation: route=" << m_currentRoute << " scenario=" << scenarioDirName;

    ui->btnStart->setEnabled(false);
    ui->btnStart->setText("Starting...");
    
    m_client->startSimulation(m_currentRoute, scenarioDirName);
}

//-----------------------------------------------------------------------------
void MainWindow::onScenarioChanged(int index)
{
    if (index < 0 || index >= ui->comboScenarios->count())
    {
        m_currentScenario.clear();
        return;
    }

    m_currentScenario = ui->comboScenarios->itemData(index).toString();
}

//-----------------------------------------------------------------------------
void MainWindow::onConnected()
{
    m_isConnected = true;
    ui->btnConnect->setText("Disconnect");
    ui->btnConnect->setChecked(true);
    ui->btnConnect->setEnabled(true);

    ui->editHost->setEnabled(false);
    ui->editPort->setEnabled(false);

    setStatus("Connected");
    updateUI();

    // Загрузка маршрутов
    m_client->loadRoutes();

    qDebug() << "Connected, loading routes...";
}

//-----------------------------------------------------------------------------
void MainWindow::onDisconnected()
{
    m_isConnected = false;
    m_isSimulationRunning = false;
    ui->btnConnect->setText("Connect");
    ui->btnConnect->setChecked(false);
    ui->btnConnect->setEnabled(true);

    ui->editHost->setEnabled(true);
    ui->editPort->setEnabled(true);

    ui->listRoutes->clear();
    ui->comboScenarios->clear();
    ui->comboScenarios->setEnabled(false);
    ui->labelRouteInfo->setText("Route info");

    setStatus("Disconnected", true);
    updateUI();
}

//-----------------------------------------------------------------------------
void MainWindow::onError(const QString& error)
{
    setStatus("Error: " + error, true);
    QMessageBox::critical(this, "Error", error);
    ui->btnStart->setEnabled(true);
}

//-----------------------------------------------------------------------------
void MainWindow::onRoutesLoaded()
{
    ui->listRoutes->clear();

    for (const RouteData& route : m_client->getRoutes())
    {
        QListWidgetItem* item = new QListWidgetItem(ui->listRoutes);
        item->setText(route.title + " (v" + route.version + ")");
        item->setData(Qt::UserRole, route.name);
        item->setToolTip(route.description);
        ui->listRoutes->addItem(item);
    }

    if (ui->listRoutes->count() > 0)
    {
        ui->listRoutes->setCurrentRow(0);
        onRouteSelected(ui->listRoutes->currentItem());
    }

    // После загрузки маршрутов запрашиваем статус
    m_client->getStatus();
    qDebug() << "Routes loaded, requesting status...";
}

//-----------------------------------------------------------------------------
void MainWindow::onScenariosLoaded()
{
    ui->comboScenarios->clear();

    QVector<ScenarioData> scenarios = m_client->getScenarios();

    if (scenarios.isEmpty())
    {
        ui->comboScenarios->addItem("No scenarios available");
        ui->comboScenarios->setEnabled(false);
        m_currentScenario.clear();
        return;
    }

    for (const ScenarioData& scenario : scenarios)
    {
        QString displayText = scenario.name;
        if (!scenario.description.isEmpty())
        {
            displayText += " - " + scenario.description;
        }
        ui->comboScenarios->addItem(displayText, scenario.name);
        ui->comboScenarios->setItemData(ui->comboScenarios->count() - 1, scenario.dirName, Qt::UserRole + 1);
    }

    ui->comboScenarios->setEnabled(true);
    if (ui->comboScenarios->count() > 0)
    {
        ui->comboScenarios->setCurrentIndex(0);
        m_currentScenario = ui->comboScenarios->itemData(0).toString();
    }

    updateUI();
}

//-----------------------------------------------------------------------------
void MainWindow::onStatusUpdated(bool running, const QString& route, const QString& scenario)
{
    m_isSimulationRunning = running;

    qDebug() << "UI status update: running=" << running 
             << "route=" << route << "scenario=" << scenario;

    if (running)
    {
        QString statusText = "Simulation running: " + route + " / " + scenario;
        setStatus(statusText);
        ui->btnStart->setText("Stop Simulation");
        ui->btnStart->setChecked(true);
        ui->btnStart->setEnabled(true);
    }
    else
    {
        setStatus("Simulation stopped");
        ui->btnStart->setText("Start Simulation");
        ui->btnStart->setChecked(false);
        ui->btnStart->setEnabled(!m_currentRoute.isEmpty() && !m_currentScenario.isEmpty());
    }

    updateUI();
}

//-----------------------------------------------------------------------------
void MainWindow::onSimulationStarted()
{
    m_isSimulationRunning = true;
    setStatus("Simulation started");
    ui->btnStart->setText("Stop Simulation");
    ui->btnStart->setChecked(true);
    ui->btnStart->setEnabled(true);
    updateUI();
    
    m_client->getStatus();
}

//-----------------------------------------------------------------------------
void MainWindow::onSimulationStopped()
{
    m_isSimulationRunning = false;
    setStatus("Simulation stopped");
    ui->btnStart->setText("Start Simulation");
    ui->btnStart->setChecked(false);
    ui->btnStart->setEnabled(true);
    updateUI();
}

//-----------------------------------------------------------------------------
void MainWindow::updateUI()
{
    bool connected = m_isConnected;
    bool running = m_isSimulationRunning;

    ui->listRoutes->setEnabled(connected && !running);
    ui->comboScenarios->setEnabled(connected && !running && ui->comboScenarios->count() > 0);
    ui->btnStart->setEnabled(connected);

    if (connected)
    {
        if (running)
        {
            ui->btnStart->setText("Stop Simulation");
            ui->btnStart->setChecked(true);
        }
        else
        {
            ui->btnStart->setText("Start Simulation");
            ui->btnStart->setChecked(false);
            ui->btnStart->setEnabled(!m_currentRoute.isEmpty() && !m_currentScenario.isEmpty());
        }
    }
    else
    {
        ui->btnStart->setText("Start Simulation");
        ui->btnStart->setChecked(false);
        ui->btnStart->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
void MainWindow::setStatus(const QString& status, bool isError)
{
    ui->labelStatus->setText(status);

    if (isError)
    {
        ui->labelStatus->setStyleSheet("font-weight: bold; color: #f44336;");
    }
    else if (m_isSimulationRunning)
    {
        ui->labelStatus->setStyleSheet("font-weight: bold; color: #4CAF50;");
    }
    else if (m_isConnected)
    {
        ui->labelStatus->setStyleSheet("font-weight: bold; color: #4a90d9;");
    }
    else
    {
        ui->labelStatus->setStyleSheet("font-weight: bold; color: #f44336;");
    }

    statusBar()->showMessage(status);
}

//-----------------------------------------------------------------------------
void MainWindow::loadScenarios(const QString& route)
{
    m_client->loadScenarios(route);
}

//-----------------------------------------------------------------------------
void MainWindow::updateRouteInfo(const RouteData& route)
{
    QString info = "<b>" + route.title + "</b><br>";
    info += "Version: " + route.version + "<br>";
    if (!route.road.isEmpty())
    {
        info += "Road: " + route.road + "<br>";
    }
    if (!route.author.isEmpty())
    {
        info += "Author: " + route.author + "<br>";
    }
    if (!route.description.isEmpty())
    {
        info += "<br>" + route.description;
    }

    ui->labelRouteInfo->setText(info);
}