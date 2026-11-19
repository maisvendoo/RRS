#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QLabel>
#include <QSplitter>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>

#include <tcp-client.h>
#include <stationmodel.h>
#include <mnemonicwidget.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    TcpClient *tcp_client = nullptr;
    tcp_config_t tcp_config;
    StationModel station_model;
    dispatcher_data_t m_disp_data;
    MnemonicWidget *mnemonic = nullptr;
    QPlainTextEdit *ptLog = nullptr;
    QLabel *lblStatus = nullptr;
    QMenu *mStations = nullptr;
    QAction *actFollowTrain = nullptr;
    int follow_vehicle = -1;

    void loadSettingsGUI();
    void loadConfig(const QString &cfg_name);
    void updateStatus(const QString &text);

private slots:
    void slotConnected();
    void slotConnectedSetup();
    void slotDisconnected();
    void slotRecvLogMessage(QString msg);
    void slotStationsData(QByteArray &data);
};

#endif