//------------------------------------------------------------------------------
//
//  Protocol handler for server
//  (c) SimulatorServer 2026
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

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class ProtocolHandler : public QObject
{
    Q_OBJECT

public:

    explicit ProtocolHandler(QTcpSocket* socket, QObject* parent = nullptr);
    ~ProtocolHandler();

    void sendMessage(const QJsonObject& message);
    void sendRouteList(const QJsonArray& routes);
    void sendScenarioList(const QJsonArray& scenarios);
    void sendError(const QString& error);
    void sendSuccess(const QString& message);
    void sendStatus(const QJsonObject& status);
    void sendAuthSuccess();
    void sendAuthFailed(const QString& reason);

    QTcpSocket* getSocket() const { return m_socket; }
    QString getClientAddress() const;
    bool isConnected() const;  // <-- ТОЛЬКО ОДНО ОБЪЯВЛЕНИЕ
    bool isAuthenticated() const;
    void setAuthenticated(bool authenticated);

signals:

    void messageReceived(const QJsonObject& message);
    void clientDisconnected();

private slots:

    void onReadyRead();
    void onDisconnected();
    void onHeartbeatTimeout();

private:

    QTcpSocket*     m_socket;
    QByteArray      m_buffer;
    QTimer*         m_heartbeatTimer;
    qint64          m_lastActivity;
    bool            m_authenticated;
};

#endif // PROTOCOLHANDLER_H