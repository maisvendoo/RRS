//------------------------------------------------------------------------------
//
//  Protocol handler for client
//  (c) SimulatorClient 2026
//
//------------------------------------------------------------------------------

#ifndef     PROTOCOLHANDLER_H
#define     PROTOCOLHANDLER_H

#include    <QObject>
#include    <QTcpSocket>
#include    <QJsonDocument>
#include    <QJsonObject>
#include    <QJsonArray>
#include    <QTimer>

class ProtocolHandler : public QObject
{
    Q_OBJECT

public:
    explicit ProtocolHandler(QObject* parent = nullptr);
    ~ProtocolHandler();

    bool connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;

    void sendMessage(const QJsonObject& message);
    void getRoutes();
    void getScenarios(const QString& route);
    void startSimulation(const QString& route, const QString& scenario);
    void stopSimulation();
    void getStatus();

signals:
    void connected();
    void disconnected();
    void error(const QString& error);

    void routesReceived(const QJsonArray& routes);
    void scenariosReceived(const QJsonArray& scenarios);
    void statusReceived(const QJsonObject& status);
    void simulationStarted();
    void simulationStopped();
    void messageReceived(const QJsonObject& message);

private slots:
    void onReadyRead();
    void onDisconnected();
    void onHeartbeat();

private:
    QTcpSocket*     m_socket;
    QByteArray      m_buffer;
    QTimer*         m_heartbeatTimer;
    QString         m_host;
    quint16         m_port;
};

#endif // PROTOCOLHANDLER_H