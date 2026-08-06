//------------------------------------------------------------------------------
//
//  Main window
//  (c) SimulatorClient 2026
//
//------------------------------------------------------------------------------

#ifndef     MAINWINDOW_H
#define     MAINWINDOW_H

#include    <QMainWindow>
#include    <QListWidgetItem>

#include    "ClientCore.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

//-----------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:

    void closeEvent(QCloseEvent* event) override;

private slots:

    void onConnectButtonClicked();
    void onRouteSelected(QListWidgetItem* item);
    void onStartButtonClicked();
    void onScenarioChanged(int index);

    void onConnected();
    void onDisconnected();
    void onError(const QString& error);

    void onRoutesLoaded();
    void onScenariosLoaded();
    void onStatusUpdated(bool running, const QString& route, const QString& scenario);
    void onSimulationStarted();
    void onSimulationStopped();

private:

    void updateUI();
    void setStatus(const QString& status, bool isError = false);
    void loadScenarios(const QString& route);
    void updateRouteInfo(const RouteData& route);
    void loadConfig();

    Ui::MainWindow*     ui;
    ClientCore*         m_client;
    QString             m_currentRoute;
    QString             m_currentScenario;
    bool                m_isConnected;
    bool                m_isSimulationRunning;
};

#endif // MAINWINDOW_H