#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QLabel>

#include <tcp-client.h>
#include <stationmodel.h>

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

    QPlainTextEdit *ptLog = nullptr;

    QLabel *lblStatus = nullptr;

    void loadSettingsGUI();

    void loadConfig(const QString &cfg_name);

    void updateStatus(const QString &text);

private slots:
    void slotConnected();
    void slotDisconnected();
    void slotRecvLogMessage(QString msg);
    void slotStationsData(QByteArray &data);
};

#endif