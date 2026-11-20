#ifndef     PROXY_SERVER_H
#define     PROXY_SERVER_H

#include    <QObject>
#include    <QTcpServer>
#include    <QTcpSocket>
#include    <QPointer>
#include    <QMap>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
// Класс прозрачного TCP-ретранслятора (прокси) между внешними клиентами
// и локальным симулятором. Каждое входящее соединение направляется
// в отдельное новое соединение к целевому хосту:порту.
//------------------------------------------------------------------------------
class ProxyServer : public QObject
{
    Q_OBJECT

public:

    ProxyServer(QObject *parent = nullptr);

    ~ProxyServer();

    /// Инициализация и запуск прокси из конфигурационного файла
    bool init(QString cfg_path);

private:

    /// Порт, на котором прокси принимает внешние соединения
    quint16 listen_port = 1993;

    /// Адрес целевого (локального) сервера
    QString target_host = "127.0.0.1";

    /// Порт целевого (локального) сервера
    quint16 target_port = 1992;

    /// Входящий сетевой сервер
    QTcpServer *server = nullptr;

    /// Соединения: внешний сокет -> целевой сокет
    QMap<QTcpSocket*, QTcpSocket*> connections;

private slots:

    /// Обработка нового входящего соединения
    void slotNewConnection();

    /// Передача данных от внешнего клиента целевому серверу
    void slotForwardToTarget();

    /// Передача данных от целевого сервера внешнему клиенту
    void slotForwardToClient();

    /// Закрытие соединения при разрыве одной из сторон
    void slotDisconnected();
};

#endif // PROXY_SERVER_H
