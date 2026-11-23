#ifndef     PROXY_SERVER_H
#define     PROXY_SERVER_H

#include    <QObject>
#include    <QTcpServer>
#include    <QTcpSocket>
#include    <QPointer>
#include    <QMap>
#include    <QVector>
#include    <QTimer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
// Класс прозрачного TCP-ретранслятора (прокси) между внешними клиентами
// и локальным симулятором. Каждое входящее соединение направляется
// в отдельное новое соединение к целевому хосту:порту.
// Обеспечивает защиту от исчерпания ресурсов: лимит одновременных
// соединений, ограничение скорости подключений, таймаут простоя
// и ограничение размера кадра.
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

    /// Максимальное число одновременных соединений
    int max_connections = 100;

    /// Максимальное число новых подключений в секунду
    int rate_limit_per_sec = 20;

    /// Таймаут простоя соединения, мс
    int idle_timeout_ms = 30000; // 30 секунд

    /// Максимальный размер кадра, байт
    quint32 max_frame_size = 1048576;

    /// Входящий сетевой сервер
    QTcpServer *server = nullptr;

    /// Соединения: внешний сокет -> целевой сокет
    QMap<QTcpSocket*, QTcpSocket*> connections;

    /// Таймеры простоя для каждого внешнего соединения
    QMap<QTcpSocket*, QTimer*> idle_timers;

    /// Отметки времени приёма новых соединений (для rate limit)
    QVector<qint64> connection_times;

    /// Время последней активности по каждому соединению
    QMap<QTcpSocket*, qint64> last_activity;

    /// Буфер данных, поступивших от клиента до завершения соединения с таргетом
    QMap<QTcpSocket*, QByteArray> pending_buffers;

    /// Сброс таймера простоя соединения
    void resetIdleTimer(QTcpSocket *client_socket);

    /// Закрытие и удаление соединения
    void closeConnection(QTcpSocket *client_socket);

    /// Проверка ограничения скорости новых подключений
    bool isRateLimited();

    /// Отправка накопленного буфера данных таргету после установки соединения
    void flushPendingBuffer(QTcpSocket *client_socket);

    /// Обновление времени последней активности соединения
    void updateActivity(QTcpSocket *client_socket);

private slots:

    /// Обработка нового входящего соединения
    void slotNewConnection();

    /// Передача данных от внешнего клиента целевому серверу
    void slotForwardToTarget();

    /// Передача данных от целевого сервера внешнему клиенту
    void slotForwardToClient();

    /// Закрытие соединения при разрыве одной из сторон
    void slotDisconnected();

    /// Закрытие соединения по таймауту простоя
    void slotIdleTimeout();
};

#endif // PROXY_SERVER_H
